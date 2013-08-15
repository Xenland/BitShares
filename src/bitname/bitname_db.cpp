#include <bts/bitname/bitname_db.hpp>
#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <bts/config.hpp>
#include <bts/db/level_map.hpp>
#include <bts/db/level_pod_map.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace bitname {

    namespace ldb = leveldb;
   
    static_assert( sizeof(bts::bitname::name_db::name_location) == 12, 
                      "name_location requires dense packing with no padding" );

    namespace detail 
    {
       class name_db_impl 
       {
          public:
             name_db_impl()
             {
             }

             db::level_pod_map<uint32_t, mini_pow>                             _block_num_to_id;
             db::level_pod_map<mini_pow, name_block>                           _block_id_to_block;
             db::level_pod_map<uint64_t, std::vector<name_db::name_location> > _name_hash_to_locs;

             name_header               _head_header;
             uint32_t                  _head_block_num;
             mini_pow                  _head_block_id;
             blockchain::time_keeper   _timekeeper;

             void index_trx( const name_db::name_location& loc, uint64_t name_hash )
             {
                auto name_locs_itr = _name_hash_to_locs.find( name_hash );
                if( name_locs_itr.valid() )
                {
                    auto name_locs = name_locs_itr.value();
                    name_locs.push_back( loc );
                    _name_hash_to_locs.store( name_hash, name_locs );
                }
                else
                {
                    _name_hash_to_locs.store( name_hash, std::vector<name_db::name_location>(1,loc) );
                }
             }
       };
    } // namespace detail

    name_db::name_db()
    :my( new detail::name_db_impl() ){}
    name_db::~name_db(){}

    void name_db::open( const fc::path& db_dir, bool create )
    { try {

       if( !fc::exists( db_dir ) )
       {
         if( !create )
         {
            FC_THROW_EXCEPTION( file_not_found_exception, "Unable to open name database ${dir}", ("dir",db_dir) );
         }
         fc::create_directories( db_dir );
       }
       my->_block_num_to_id.open( db_dir / "block_num_to_id" );
       my->_block_id_to_block.open( db_dir / "block_id_to_block" );
       my->_name_hash_to_locs.open( db_dir / "name_hash_to_locs" );

       auto genesis = create_genesis_block(); 
       my->_timekeeper.configure( genesis.utc_sec, fc::seconds( BITNAME_BLOCK_INTERVAL_SEC ), BITNAME_TIMEKEEPER_WINDOW );

       if( my->_block_num_to_id.last(my->_head_block_num, my->_head_block_id) )
       {
           uint32_t start_window = 0;
           if( my->_head_block_num > BITNAME_TIMEKEEPER_WINDOW ) 
           {
              start_window = my->_head_block_num - BITNAME_TIMEKEEPER_WINDOW;
           }

           for( uint32_t i = start_window; i <= my->_head_block_num; ++i )
           {
              auto block_id = my->_block_num_to_id.fetch(i);
              auto block_data = my->_block_id_to_block.fetch(block_id);
              my->_timekeeper.push_init( i, block_data.utc_sec, block_data.calc_difficulty() ); 
           }
       }
       else // no data in db, populate it with the genesis block!
       {
           my->_head_block_id  = genesis.id();
           my->_head_block_num = 0;
           my->_block_num_to_id.store( 0, genesis.id() );
           my->_block_id_to_block.store( genesis.id(), genesis );
           my->_timekeeper.push_init( 0, genesis.utc_sec, genesis.calc_difficulty() );
       }
       my->_timekeeper.init_stats();

    } FC_RETHROW_EXCEPTIONS( warn, "unable to open name db at path ${path}", ("path", db_dir)("create",create) ) }

    void name_db::close()
    {
       my->_block_num_to_id.close();
       my->_block_id_to_block.close();
       my->_name_hash_to_locs.close();
    }
    uint64_t name_db::target_difficulty()const
    {
      return my->_timekeeper.next_difficulty();
    }

    void name_db::push_block( const name_block& next_block )
    { try {
       FC_ASSERT( next_block.prev == my->_head_block_id, "", ("head_block_id",my->_head_block_id) );
       auto mroot = next_block.calc_merkle_root();
       FC_ASSERT( next_block.mroot == mroot );

       auto block_difficulty = next_block.calc_difficulty();
       FC_ASSERT( block_difficulty >= my->_timekeeper.next_difficulty() );

       size_t num_trx =  next_block.registered_names.size();
       for( uint32_t trx_idx = 0; trx_idx < num_trx; ++trx_idx )
       {
          validate_trx( next_block.registered_names[trx_idx] );
       }

       // TODO: If something fails during this operation, we need to make sure
       // that the name_db is left in the prior state.

       // if we get this far, then all trx are valid, we can store it in the db 
       // and update all of the trx indexes
       auto next_id = next_block.id();
       my->_head_block_num++;
       my->_head_block_id = next_id;
       my->_block_id_to_block.store(next_id,next_block);
       my->_block_num_to_id.store( my->_head_block_num, next_id );
       for( uint16_t trx_idx = 0; trx_idx < num_trx; ++trx_idx )
       {
          my->index_trx( name_location( next_id, trx_idx ), next_block.registered_names[trx_idx].name_hash );
       }
       my->_timekeeper.push( my->_head_block_num, next_block.utc_sec, block_difficulty );
    } FC_RETHROW_EXCEPTIONS( warn, "unable to push block ${next_block}", ("next_block", next_block) ) } 

    /**
     *  checks to make sure the transaction is valid and could be applied to the
     *  current database.
     */
    void name_db::validate_trx( const name_trx& trx )const
    { try {
       auto prev_reg_itr = my->_name_hash_to_locs.find( trx.name_hash );
       if( prev_reg_itr.valid() )
       {
          name_trx prev_trx = fetch_trx( trx.name_hash );
          FC_ASSERT( trx.renewal.value == prev_trx.renewal.value + 1, "", ("prev_trx",prev_trx) );
          // cannot renew if last renewal was a cancelation.
          FC_ASSERT( !prev_trx.cancel_sig );
          if( trx.key )
          {
              // cannot renew to new key
              FC_ASSERT( *prev_trx.key ==  *trx.key );
          }
          else
          {
              FC_ASSERT( !!trx.cancel_sig ); // must have sig to cancel previous reg.
              // TODO: verify signature is of the prev_trx and the signer is the prev_trx.key
              FC_ASSERT( !"TODO: Cancelation Not Implemented Yet" );
          }
          FC_ASSERT( trx.utc_sec > my->_head_header.utc_sec );
       }
       else
       {
          FC_ASSERT( trx.renewal == 0 );
       }
    } FC_RETHROW_EXCEPTIONS( warn, "error validating ${trx}", ("trx", trx) ) }
   
    void name_db::pop_block()
    {
      FC_ASSERT( !"TODO: pop_block Not implemented" );
    }
    
    mini_pow name_db::head_block_id()const 
    { 
      return my->_head_block_id; 
    }


    name_db::name_location name_db::find_name( uint64_t name_hash )const
    { try {

        auto name_locations = my->_name_hash_to_locs.fetch(name_hash);
        FC_ASSERT( name_locations.size() > 0 );
        return name_locations.back();

    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch location for name with hash ${name_hash}", ("name_hash",name_hash) ) }


    name_trx   name_db::fetch_trx( uint64_t name_hash )const
    { try {
        auto name_loc = find_name( name_hash );
        auto name_block = fetch_block( name_loc.block_id );
        FC_ASSERT( name_block.registered_names.size() > name_loc.trx_num );
        FC_ASSERT( name_block.registered_names[name_loc.trx_num].name_hash == name_hash );
        return name_block.registered_names[name_loc.trx_num];
    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch trx for name hash ${name_hash}", ("name_hash", name_hash ) ) }

    name_block   name_db::fetch_block( const mini_pow& block_id )const
    { try {
        return my->_block_id_to_block.fetch(block_id);
    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch block id ${block_id}", ("block_id",block_id) ) }



} } // bts::bitname
