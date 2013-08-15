#pragma once
#include <bts/bitname/bitname_block.hpp>
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

        void push_block( const name_block& b );
        void pop_block(); // pops the most recent block

        mini_pow head_block_id()const;
        uint64_t target_difficulty()const;

        struct name_location 
        {
            name_location( const mini_pow& i, uint16_t n )
            :block_id(i),trx_num(n){}
            name_location():trx_num(0){}

            mini_pow block_id;
            uint16_t trx_num;
        };
        
        /**
         *  Checks to see if the name can be registered and
         *  throws an exception on error.
         */
        void validate_trx( const name_trx& trx )const;

        /** finds the location of most recent registration of name_hash */
        name_location   find_name( uint64_t name_hash )const;

        /** fetches the most recent registration of name_hash */
        name_trx        fetch_trx( uint64_t name_hash )const;

        /** get a block by its block_id */
        name_block      fetch_block( const mini_pow& block_id )const;

      private:
        std::unique_ptr<detail::name_db_impl> my;
  };

} }

FC_REFLECT( bts::bitname::name_db::name_location, (block_id)(trx_num) )
