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

        /**
         *  Push the block, validating it, and throw an exception
         *  if there are any problems. todo: validate all trxtimes
         *  are within 10 minutes of the block time
         */
        void push_block( const name_block& b );
        /**
         *  Checks to see if the name can be registered and
         *  throws an exception on error.
         */
        void validate_trx( const name_trx& trx, bool is_header = false )const;

        void pop_block(); // pops the most recent block

        uint32_t             head_block_num()const;
        name_id_type         head_block_id()const;
        uint64_t             target_difficulty()const;
        uint64_t             target_name_difficulty()const;
        fc::time_point_sec   chain_time()const;

        /**
         *   The cumulative difficulty of the chain
         */
        uint64_t             chain_difficulty()const;

        /**
         * Tracks the ID of all headers for rapid query by new nodes
         * connecting and attempting to sync.
         */
        const std::vector<name_id_type>&  get_header_ids()const;

        /** Returns the number of blocks that have confirmed the
         *  most recent name.
         */
        uint32_t        get_expiration( uint64_t name_hash )const;

        /** fetches the most recent registration of name_hash */
        name_trx        fetch_trx( uint64_t name_hash )const;
        uint32_t        fetch_repute( uint64_t name_hash )const;

        /** get a block by its block_id */
        name_block      fetch_block( const name_id_type& block_id )const;
        name_block      fetch_block( uint32_t block_num )const;

        name_header     fetch_block_header( const name_id_type& block_id )const;
        name_header     fetch_block_header( uint32_t block_num )const;
        uint32_t        get_block_num( const name_id_type& block_id )const;

        /** the time we expect block_num to be generated */
        fc::time_point_sec expected_time( uint32_t block_num )const;

        void            dump();
      private:
        std::unique_ptr<detail::name_db_impl> my;
  };

} }

