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
       asset  fees; // any fees that would be generated
       asset  coinbase; // how many new bitshares are created by a trx.
       trx_eval& operator += ( const trx_eval& e )
       {
         fees += e.fees;
         coinbase += e.coinbase;
         return *this;
       }
    };

    struct trx_num
    {
      /** 
       *  -1 block_num is used to identifiy default initialization.
       */
      static const uint32_t invalid_block_id = -1;
      trx_num(uint32_t b = invalid_block_id, uint16_t t = 0):block_num(b),trx_idx(t){}
      uint32_t block_num;
      uint16_t trx_idx;

      friend bool operator < ( const trx_num& a, const trx_num& b )
      {
        return a.block_num == b.block_num ? 
                    a.trx_idx < b.trx_idx : 
                    a.block_num < b.block_num;
      }
      friend bool operator == ( const trx_num& a, const trx_num& b )
      {
        return a.block_num == b.block_num && a.trx_idx == b.trx_idx;
      }
    };

    /**
     *  Meta information maintained for each output that links it
     *  to the block, trx, and output
     */
    struct meta_trx_output
    {
       meta_trx_output()
       :input_num(-1){}
       trx_num   trx_id;
       uint8_t   input_num;

       bool is_spent()const 
       {
         return trx_id.block_num != trx_num::invalid_block_id;
       }
    };

    /**
     *  Caches output information used by inputs while
     *  evaluating a transaction.
     */
    struct meta_trx_input
    {
       trx_num           source;
       uint8_t           output_num;
       trx_output        output;
       meta_trx_output   meta_output;
    };

    struct meta_trx : public signed_transaction
    {
       meta_trx(){}
       meta_trx( const signed_transaction& t )
       :signed_transaction(t){}
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

          uint32_t head_block_num()const;

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
         trx_eval   evaluate_signed_transaction( const signed_transaction& trx );       

         trx_num    fetch_trx_num( const uint160& trx_id );
         meta_trx   fetch_trx( const trx_num& t );

         /**
          *  @return the outputs referenced by the inputs
          *  @throw if any input cannot be found.
          */
         std::vector<meta_trx_input> fetch_inputs( const std::vector<trx_input>& inputs );

         uint32_t   fetch_block_num( const fc::sha224& block_id );
         block      fetch_block( uint32_t block_num );
         full_block fetch_block_trxs( uint32_t block_num );

         /**
          *  Calculate the dividends due to a given asset accumulated durrning blocks from_num to to_num
          *
          *  @return only the dividends, not the balance including a
          */
         asset      calculate_dividends( const asset& a, uint32_t from_num, uint32_t to_num );

         /**
          *  The most recent blocks do not pay dividends, except to the miner, becaues the dividends
          *  would be lost in a chain reorg.  
          *
          *  @return only the dividends, not the balance
          */
         asset      calculate_dividend_fees( const asset& b, uint32_t from_num );

         /**
          *  Returns all dividends due to an output with balance b in block from_num not
          *  including dividends from the last 100 blocks.
          *
          *  @return only the dividends paid, not including the initial balance
          */
         asset      calculate_output_dividends( const asset& b, uint32_t from_num );
         
         /**
          *  Attempts to append block b to the block chain with the given trxs.
          */
         void push_block( const trx_block& b );

         /**
          *  Removes the top block from the stack and marks all spent outputs as 
          *  unspent.
          */
         void pop_block( full_block& b, std::vector<signed_transaction>& trxs );

       private:
         void   store_trx( const signed_transaction& trx, const trx_num& t );
         std::unique_ptr<detail::blockchain_db_impl> my;          
    };

}  } // bts::blockchain

FC_REFLECT( bts::blockchain::trx_eval, (fees)(coinbase) )
FC_REFLECT( bts::blockchain::trx_num, (block_num)(trx_idx) );
FC_REFLECT( bts::blockchain::meta_trx_output, (trx_id)(input_num) )
FC_REFLECT( bts::blockchain::meta_trx_input, (source)(output_num)(output)(meta_output) )
FC_REFLECT_DERIVED( bts::blockchain::meta_trx, (bts::blockchain::signed_transaction), (meta_outputs) );
