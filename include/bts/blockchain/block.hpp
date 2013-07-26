#pragma once
#include <bts/proof.hpp>
#include <bts/blockchain/transaction.hpp>

namespace bts
{
   /**
    *  Light-weight summary of a block that links it to
    *  all prior blocks.  This summary does not contain
    *  the nonce because that information is provided by
    *  the block_proof struct which is a header plus 
    *  proof of work.   
    */
   struct block_header
   {
      block_header()
      :version(0),block_num(0),timestamp(0){}

      uint16_t   version;
      fc::sha224 prev;
      uint32_t   block_num;
      uint32_t   timestamp;   ///< seconds from 1970
      fc::sha224 state_hash;  ///< hash of the block state.
   };

   /**
    *  This is the minimum subset of data that must be kept to
    *  preserve the proof of work history.
    */
   struct block_proof : public block_header
   {
      proof        pow;
   };

   /**
    *  Data managed by the output table necessary to 
    *  recreate an entry if this block is unapplied.
    */
   struct source_output  
   {
      source_output()
      :table_idx(0),block_num(0){}

      uint32_t         table_idx; // index in the dds table
      uint32_t         block_num; // 4 bytes
      generic_trx_out  output;    // 1 + N bytes
   };

   /**
    *  This is the contents of the block 
    */
   struct block_state
   {
      uint16_t version;
      fc::sha224 pre_dds_state;
      fc::sha224 post_dds_state;

      /**
       *  Contains information about what features are supported
       *  by a particular miner.  By including this in the block
       *  state the network can track upgrades and enable new features
       *  once X% of all miners support the feature.
       */
      std::vector<std::string>        miner_features;

      std::vector<signed_transaction> transactions;
      /**
       *  Ordered in the same manner as transactions and their
       *  outputs Trx#.OUT#.  Ie:  
       *
       *   Trx 0.0 = 0
       *   Trx 0.1 = 1
       *   Trx 1.0 = 2
       *   Trx 1.1 = 4
       *   Trx 1.2 = 5
       *   ...
       */
      std::vector<uint32_t>           output_indexes;

      /** tracks the outputs that are consumed by the transactions
       *  in this block so that they may be reversed.
       */
      std::vector<source_output>      sources;

      /**
       *  Year old dividends that are only stored so that this
       *  they may restored when unapplying this block.
       */
      std::vector<uint64_t>           expired_dividends;
   };


   /**
    *  This is the complete block including all transactions, 
    *  and the proof of work.
    */
   struct block : public block_proof
   {
     block_state state;
   };

}

FC_REFLECT( bts::block_header, (version)(prev)(block_num)(timestamp)(state_hash) )
FC_REFLECT_DERIVED( bts::block_proof, (bts::block_header), (pow) )
FC_REFLECT( bts::source_output, (table_idx)(block_num)(output) )
FC_REFLECT( bts::block_state, 
    (version)
    (pre_dds_state)
    (post_dds_state)
    (miner_features)
    (transactions)
    (output_indexes)
    (sources)
    (expired_dividends) )
FC_REFLECT_DERIVED( bts::block, (bts::block_proof), (state) )
