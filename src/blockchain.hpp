#pragma once
#include "units.hpp"
#include <bts/address.hpp>
#include "proof_of_work.hpp"
#include "transaction.hpp"
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/io/varint.hpp>


/**
 *  Tracks the ratio of issue to backing.  This is meant to be kept
 *  in a 64x64 array where the rows and colums corispond to unit_type.
 */
struct share_state
{
    uint64_t total_issue;
    uint64_t total_backing;
};
FC_REFLECT( share_state, (total_issue)(total_backing) )


/**
 *  Annotates the trx_output with relevant information such as which block contained the
 *  output which will be used to calculate dividends.
 */
template<typename T>
struct trx_output_cache : public T
{
   trx_output_cache():coinage(0){}
   uint32_t coinage;  ///< the block that included this output.
};

typedef trx_output_cache<trx_output_by_address> trx_output_cache_by_address;
FC_REFLECT_DERIVED( trx_output_cache_by_address, (trx_output_by_address), (coinage) )

/**
 *  Hash of all unspent outputs by type, dividend balances by coinage and type.  This
 *  structure is hashed in 8 MB chunks and those hashes are then hashed into the bstate
 *  included in the block_header
 */
struct block_state
{
   pow_hash                                    prev_block;
   ///< the block number that this state refers to.
   uint64_t                                    block_number;      

   ///< hash of all transactions that connect this state to the previous state
   fc::sha224                                  new_trxs;             

   /// indexes in unspent_by_address that are 'free'
   std::vector<uint64_t>                       freestack_by_address; 

   /// 8 units * 8 backings * 8 bytes * blocks/year =  51 MB
   std::vector<uint64_t>                       dividend_table;       

   std::vector< trx_output_cache_by_address >  unspent_by_address;   ///< 30 bytes * 100 M = 3 GB 
};
FC_REFLECT( block_state, 
  (prev_block)
  (block_number)
  (new_trxs)
  (freestack_by_address)
  (dividend_table)
  (unspent_by_address) )

/**
 *  Block headers must be kept for 1 year after which one header can
 *  be dropped for each new block added.  The block header references
 *  the hash of an 'initial condition' that must be known in order 
 *  to build new blocks from this header.  Clients only need to
 *  store transactions for enough blocks that they can be confident
 *  in moving to the new initial condition.
 *
 *  In order to ensure that all data can be discarded after one year,
 *  all unspent outputs pay a 5% tax or minimum transaction fee if
 *  they are over 1 year old and also forfeit all dividends earned.
 *
 *  In this way the network is not forced to store outputs from
 *  private keys and 'dust' forever.
 */
struct block_header
{
   block_header():height(0){}

   uint32_t                         version;
   /** 64bit UTC microseconds from 1970, must always increase from one block to the next, 
     *   blocks are rejected if timestamp is more than 5 minutes in the future.
     *   TODO: study timeing attacks on the network based upon lessons of BTC
     */
   uint32_t                         timestamp;  
   uint32_t                         height;     /// position in the block chain... 
   pow_hash                         prev_block; /// hash of previous block.
   fc::sha224                       bstate;     /// @see block_state - used to 'capture' the status of the block after procesing trx
};
FC_REFLECT( block_header, (version)(timestamp)(height)(prev_block)(bstate) )



/**
 * 
 */
struct pow_block_header 
{
   /**
    *   Incremented and hashed with mbranch to
    *   prove work.  
    */
   uint64_t                nonce;
   /** the first hash is hashed with nonce to prove
    *  the work.
    *
    *  The last hash is the hash of the block_header and
    *  everything inbetween are the series of hashes in
    *  the merkel tree that connects the header hash to
    *  the root hash.
    */ 
   std::vector<fc::sha224> mbranch; 

   /// this is hashed to create the tail of the mbranch
   block_header            blockhead;
};
FC_REFLECT( pow_block_header, (nonce)(mbranch)(blockhead) )


/**
 *  Combines the block header with the transactions that 
 *  comprise the block.  When these transactions are applied
 *  to the previous block's state they should yield the
 *  current block's state.
 */
struct block
{
   pow_block_header          header;
   std::vector<transaction>  trxs;
};
FC_REFLECT( block, (header)(trxs) )


