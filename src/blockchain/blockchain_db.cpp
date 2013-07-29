#include <bts/blockchain/blockchain_db.hpp>
#include <leveldb/db.h>
#include "../db.hpp"

#include <fc/filesystem.hpp>

namespace bts { namespace blockchain {
    namespace ldb = leveldb;
    namespace detail  
    { 
      class blockchain_db_impl
      {
         public:
            std::unique_ptr<ldb::DB> blk_id2num;  // maps blocks to unique IDs
            std::unique_ptr<ldb::DB> blocks;      // block num to block struct
            std::unique_ptr<ldb::DB> block_trxs;  // bluck num to trx hashes
            std::unique_ptr<ldb::DB> trx_id2num;  // maps trxs to unique numbers
            std::unique_ptr<ldb::DB> trxs;        // trxnum to trx;


            // Dividend Table needs to be memory mapped

      };
    }

     blockchain_db::blockchain_db()
     :my( new detail::blockchain_db_impl() )
     {
     }

     blockchain_db::~blockchain_db()
     {
     }

     void blockchain_db::open( const fc::path& dir, bool create )
     {
        if( !fc::exists( dir ) )
        {
             if( !create )
             {
                FC_THROW_EXCEPTION( file_not_found_exception, 
                    "Unable to open name database ${dir}", ("dir",dir) );
             }
             fc::create_directories( dir );
        }
        my->blk_id2num = init_db( dir / "blk_id2num", create );
        my->blocks     = init_db( dir / "blocks",     create );
        my->block_trxs = init_db( dir / "block_trxs", create );
        my->trx_id2num = init_db( dir / "trx_id2num", create );
        my->trxs       = init_db( dir / "trxs",       create );

     }
     void blockchain_db::close()
     {
        my->blk_id2num.reset();
        my->blocks.reset();
        my->block_trxs.reset();
        my->trx_id2num.reset();
        my->trxs.reset();
     }

    /**
     *  Validates that trx could be included in a future block, that
     *  all inputs are unspent, that it is valid for the current time,
     *  and that all inputs have proper signatures and input data.
     *
     *  @return any trx fees that would be paid if this trx were included
     *          in the next block.
     *
     *  @throw exception if trx can not be applied to the current chain state.
     */
    trx_eval blockchain_db::evaluate_signed_transaction( const signed_transaction& trx )       
    {
    }

    void               blockchain_db::store_trx( const signed_transaction& trx )
    {
    }
    meta_trx           blockchain_db::fetch_trx( const uint160& trx_id )
    {
    }

    uint32_t           blockchain_db::fetch_block_num( const fc::sha256& block_id )
    {
    }
    block         blockchain_db::fetch_block( uint32_t block_num )
    {
    }
    full_block              blockchain_db::fetch_block_trxs( uint32_t block_num )
    {
    }

    /**
     *  Calculate the dividends due to a given asset accumulated durrning blocks from_num to to_num
     */
    asset              blockchain_db::calculate_dividends( const asset& a, uint32_t from_num, uint32_t to_num )
    {
    }
    
    /**
     *  Attempts to append block b to the block chain with the given trxs.
     */
    void blockchain_db::push_block( const full_block& b, const std::vector<signed_transaction>& trxs )
    {
    }

    /**
     *  Removes the top block from the stack and marks all spent outputs as 
     *  unspent.
     */
    void blockchain_db::pop_block( full_block& b, std::vector<signed_transaction>& trxs )
    {
    }

}  } // bts::blockchain
