#pragma once
#include <bts/blockchain/block.hpp>
#include <bts/blockchain/transaction.hpp>

namespace fc 
{
   class path;
};

namespace bts { namespace blockchain {

    namespace detail  { class blockchain_db_impl; }

    /**
     *  Information generated as a result of evaluating a signed
     *  transaction.
     */
    struct trx_eval
    {
       asset              fees; // any fees that would be generated
    };

    /**
     *  Meta information maintained for each output that links it
     *  to the block, trx, and output
     */
    struct meta_trx_output
    {
       uint32_t  block_num;
       uint16_t  trx_num;
       uint8_t   input_num;
    };

    struct meta_trx : public signed_transaction
    {
       uint32_t block_num; // the block that contains the trx;
       std::vector<meta_trx_output> meta_outputs; // tracks where the output was spent
    };

    /**
     *  This database only stores valid blocks and applied transactions,
     *  it does not store invalid/orphaned blocks and transactions which
     *  are maintained in a separate database 
     */
    class blockchain_db 
    {
       public:
          blockchain_db();
          ~blockchain_db();

          void open( const fc::path& dir, bool create = true );
          void close();

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
         trx_eval evaluate_signed_transaction( const signed_transaction& trx );       

         void               store_trx( const signed_transaction& trx );
         meta_trx           fetch_trx( const uint160& trx_id );

         uint32_t           fetch_block_num( const fc::sha256& block_id );
         block_proof        fetch_block_proof( uint32_t block_num );
         block_state        fetch_block_state( uint32_t block_num );
         full_block         fetch_block( uint32_t block_num );

         /**
          *  Calculate the dividends due to a given asset accumulated durrning blocks from_num to to_num
          */
         asset              calculate_dividends( const asset& a, uint32_t from_num, uint32_t to_num );
         
         /**
          *  Attempts to append block b to the block chain with the given trxs.
          */
         void push_block( const full_block& b, const std::vector<signed_transaction>& trxs );

         /**
          *  Removes the top block from the stack and marks all spent outputs as 
          *  unspent.
          */
         void pop_block( full_block& b, std::vector<signed_transaction>& trxs );

       private:
         std::unique_ptr<detail::blockchain_db_impl> my;          
    };

}  } // bts::blockchain

FC_REFLECT( bts::blockchain::trx_eval, (fees) )
FC_REFLECT( bts::blockchain::meta_trx_output, (block_num)(trx_num)(input_num) )
FC_REFLECT_DERIVED( bts::blockchain::meta_trx, (bts::blockchain::signed_transaction), (block_num)(meta_outputs) );
