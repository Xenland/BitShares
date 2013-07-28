#pragma once
#include <bts/proof.hpp>
#include <bts/blockchain/transaction.hpp>
#include <bts/blockchain/bitassets.hpp>

namespace bts { namespace blockchain {
   typedef fc::sha1 uint160;

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

      fc::unsigned_int    version;
      mini_pow            prev;
      uint32_t            block_num;
      fc::time_point_sec  timestamp;   ///< seconds from 1970
      fc::uint160         state_hash;  ///< ripemd160(  sha512( block_state ) )
      fc::uint160         trx_mroot;   ///< merkle root of trx included in block, required for light client validation
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
    *  Tracks the ratio of bitshares to issued bit-assets.
    */
   struct asset_issuance 
   {
      uint128_t  backing; /** total BitShares backing the issued currency */
      uint128_t  issued;  /** total asset issued */
   };


   /**
    *  Block state is maintained so that the initial condition of the 
    *  1 year old block can be known without having to have the full history. 
    *
    *  Storing the block state with every block for 1 year is about 100MB
    */
   struct block_state
   {
      /** initial condition prior to applying trx in this block */
      fc::array<bitasset_type::count,asset_issuance> issuance;  // 16 * 32 bytes = 512

      /**
       *  Features desired / supported by the miner. Once 75% of the past week worth
       *  of blocks supports a feature miner may start generating blocks that use
       *  the new feature / rule changes and anyone on the minority chain will
       *  be alerted that they no longer support the main chain.
       */
      std::vector<fc::unsigned_int>        supported_features;
   };


   /**
    *  This is the complete block including all transactions, 
    *  and the proof of work.
    */
   struct block : public block_proof
   {
     block_state state;
   };

   /**
    * A block complete with the IDs of the transactions included
    * in the block.
    */
   struct full_block : public block 
   {
      std::vector<fc::uint160>  trx_ids; 
   };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::asset_issuance,      (backing)(issued) )
FC_REFLECT( bts::blockchain::block_state,         (issuance)(supported_features) )
FC_REFLECT( bts::blockchain::block_header,        (version)(prev)(block_num)(timestamp)(state_hash)(trx_mroot) )
FC_REFLECT_DERIVED( bts::blockchain::block_proof, (bts::blockchain::block_header), (pow)     )
FC_REFLECT_DERIVED( bts::blockchain::block,       (bts::blockchain::block_proof),  (state)   )
FC_REFLECT_DERIVED( bts::blockchain::full_block,  (bts::blockchain::block),        (trx_ids) )
