#include <bts/bitname/name_db.hpp>
#include <leveldb/db.h>
#include "../db.hpp"
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
            std::unique_ptr<ldb::DB> name_index_db;
            std::unique_ptr<ldb::DB> name_block_db;
       };
    }

    name_db::name_db()
    :my( new detail::name_db_impl() ){}
    name_db::~name_db(){}

    void name_db::open( const fc::path& db_dir, bool create )
    {
       if( !fc::exists( db_dir ) )
       {
         if( !create )
         {
            FC_THROW_EXCEPTION( file_not_found_exception, "Unable to open name database ${dir}", ("dir",db_dir) );
         }
         fc::create_directories( db_dir );
       }
       auto nidx = db_dir / "name_idx";
       auto nblk = db_dir / "blocks";

       my->name_index_db = init_db( db_dir / "name_idx", create ); 
       my->name_block_db = init_db( db_dir / "blocks", create ); 

    }
    void name_db::close()
    {
       my->name_index_db.reset();
       my->name_block_db.reset();
    }

    void name_db::store( const name_block& b )
    {
       auto bid = b.id();
       ldb::Slice key( bid.data, sizeof(bid) );

       auto bvec = fc::raw::pack( b );
       ldb::Slice block_data( bvec.data(), bvec.size() );

       auto status = my->name_block_db->Put( ldb::WriteOptions(), key, block_data );

       if( !status.ok() )
       {
          FC_THROW_EXCEPTION( exception, "Unable to store name block ${b}\n\t${msg}", 
                ("b",b)("msg",status.ToString() ) );
       }

       // index the names in the block
       for( uint16_t i = 0; i < b.registered_names.size(); ++i )
       {
          name_location l(bid,i);
          ldb::Slice value( (char*)&l, sizeof(l) );
          ldb::Slice nhash( (char*)&b.registered_names[i].name_hash, 8 );
          status = my->name_index_db->Put( ldb::WriteOptions(), nhash, value );
          if( !status.ok() )
          {
              FC_THROW_EXCEPTION( exception, "Unable to store name index ${l}\n\t${msg}", 
                    ("l",l)("msg",status.ToString() ) );
          }
       }
    }

    name_db::name_location name_db::find_name( uint64_t name_hash )
    {
       std::string val;
       ldb::Slice  key( (char*)&name_hash, sizeof(name_hash) );
       my->name_index_db->Get( ldb::ReadOptions(), key, &val );
       name_location loc;
       memcpy( (char*)&loc, val.c_str(), val.size());
       return loc;
    }


    name_header   name_db::fetch_trx( uint64_t name_hash )
    {
       try {
          auto loc = find_name( name_hash );
          name_block  b = fetch_block( loc.block_id );
          FC_ASSERT( b.registered_names.size() > loc.trx_num );
          return name_header(b.registered_names[loc.trx_num], b.prev );
       } FC_RETHROW_EXCEPTIONS( warn, "Unable to fetch name registration for '${name}'", ("name",name_hash) );
    }

    name_block   name_db::fetch_block( const mini_pow& block_id )
    {
       try {
          ldb::Slice id( block_id.data, sizeof(block_id) );
          std::string value;
          auto status = my->name_block_db->Get( ldb::ReadOptions(), id, &value );
          if( !status.ok() )
          {
             FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
          }

          name_block blk;
          fc::datastream<const char*> ds( value.c_str(), value.size() );
          fc::raw::unpack( ds, blk );
          return blk;
       } FC_RETHROW_EXCEPTIONS( warn, "Unable to fetch block with id '${id}'", ("id",block_id) );
    }


    void name_db::remove_block( const mini_pow& block_id )
    {
        try
        {
            name_block blk = fetch_block( block_id );
            for( auto itr = blk.registered_names.begin(); itr != blk.registered_names.end(); ++itr )
            {
               if( find_name( itr->name_hash ).block_id == block_id )
               {
                  ldb::Slice name_key( (char*)&itr->name_hash, sizeof(itr->name_hash) );
                  auto status = my->name_index_db->Delete( ldb::WriteOptions(), name_key );
                  if( !status.ok() )
                  {
                    FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
                  }
               }
            }
            ldb::Slice id( block_id.data, sizeof(block_id) );
            auto status = my->name_block_db->Delete( ldb::WriteOptions(), id );
            if( !status.ok() )
            {
              FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
            }

        }FC_RETHROW_EXCEPTIONS( warn, "error removing name block ${block_id}", ("block_id",block_id) );
    }


} } // bts::bitname
