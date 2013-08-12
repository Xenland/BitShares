#include <bts/bitname/bitname_db.hpp>
#include <bts/db/level_map.hpp>
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
             :_head_block_num(0)
             {
             }

             db::level_map<uint32_t, mini_pow>                             _block_num_to_id;
             db::level_map<mini_pow, name_block>                           _block_id_to_block;
             db::level_map<uint64_t, std::vector<name_db::name_location> > _name_hash_to_locs;

             mini_pow _head_block_id;
             uint32_t _head_block_num;
       };
    }

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

    } FC_RETHROW_EXCEPTIONS( warn, "unable to open name db at path ${path}", ("path", db_dir)("create",create) ) }

    void name_db::close()
    {
       my->_block_num_to_id.close();
       my->_block_id_to_block.close();
       my->_name_hash_to_locs.close();
    }

    void name_db::push_block( const name_block& b )
    {
      
    }
   
    void name_db::pop_block()
    {
    }

    name_db::name_location name_db::find_name( uint64_t name_hash )
    { try {
        auto name_locations = my->_name_hash_to_locs.fetch(name_hash);
        FC_ASSERT( name_locations.size() > 0 );
        return name_locations.back();
    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch location for name with hash ${name_hash}", ("name_hash",name_hash) ) }


    name_trx   name_db::fetch_trx( uint64_t name_hash )
    { try {
        auto name_loc = find_name( name_hash );
        auto name_block = fetch_block( name_loc.block_id );
        FC_ASSERT( name_block.registered_names.size() > name_loc.trx_num );
        FC_ASSERT( name_block.registered_names[name_loc.trx_num].name_hash == name_hash );
        return name_block.registered_names[name_loc.trx_num];
    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch trx for name hash ${name_hash}", ("name_hash", name_hash ) ) }

    name_block   name_db::fetch_block( const mini_pow& block_id )
    { try {
        return my->_block_id_to_block.fetch(block_id);
    } FC_RETHROW_EXCEPTIONS( warn, "unable to fetch block id ${block_id}", ("block_id",block_id) ) }



} } // bts::bitname
