#pragma once
#include <bts/bitname/name_block.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace bitname {

  namespace detail { class name_db_impl; }

  /**
   *  Stores the valid set of blocks and maintains an
   *  index from name_hash to block/index location.
   */
  class name_db
  {
      public:
        name_db();
        ~name_db();

        void open( const fc::path& dbdir, bool create = true );
        void close();

        void store( const name_block& b );

        struct name_location 
        {
            name_location( const mini_pow& i, uint16_t n )
            :block_id(i),trx_num(n){}
            name_location():trx_num(0){}

            mini_pow block_id;
            uint16_t trx_num;
        };

        name_location   find_name( uint64_t name_hash );
        name_header     fetch_trx( uint64_t name_hash );
        name_block      fetch_block( const mini_pow& block_id );

        void            remove_block( const mini_pow& block_id );

      private:
        std::unique_ptr<detail::name_db_impl> my;
  };

} }

FC_REFLECT( bts::bitname::name_db::name_location, (block_id)(trx_num) )
