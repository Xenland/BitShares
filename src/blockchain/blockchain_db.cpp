#include <bts/blockchain/blockchain_db.hpp>

namespace bts { namespace blockchain {

    namespace detail  
    { 
      class blockchain_db_impl
      {
         public:
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
     }
     void blockchain_db::close()
     {
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
    full_block         blockchain_db::fetch_block( uint32_t block_num )
    {
    }
    block_proof        blockchain_db::fetch_block_proof( uint32_t block_num )
    {
    }
    block_state        blockchain_db::fetch_block_state( uint32_t block_num )
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
