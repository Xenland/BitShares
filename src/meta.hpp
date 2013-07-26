#pragma once
/**
 *  @file meta.hpp
 *
 *  Defines extra 'index' information about transactions, outputs, and blocks.  This
 *  information is not included in the blockchain but is kept in the local database.
 */
#include "blockchain.hpp"
#include "chain_state.hpp"
#include <fc/crypto/sha1.hpp>

/**
 *  Tracks information / state about a transactions.
 */
class meta_transaction
{
    public:
      transaction             trx;       ///< the actual transaction 
      std::string             memo;      ///< user-specified memo regarding this trx
      std::string             error;     ///< any error message regarding this transaction
      int32_t                 block_num; ///< if this trx is in the valid chain, this will be set.
      std::vector<fc::sha256> blocks;    ///< all blocks that include this trx
};

FC_REFLECT( meta_transaction, (trx)(memo)(error)(block_num)(blocks) )


class meta_output_cache
{
   public:
      output_cache out;
      int32_t      block_num;    ///< non 0 if included in a block
      fc::sha224   trx_id;       /// the ID of the transaction this output was part of
      fc::sha224   spent_trx_id; /// trx that spent this output
};

FC_REFLECT( meta_output_cache, (out)(block_num)(trx_id)(spent_trx_id) )

class meta_block_header
{
  public:
    fc::sha1                  id;               // cached because it is expensive to calculate 
    fc::string                error_message;
    block_header              header;
    chain_state::change_set   undo_data;        // everything necessary to 'undo' state changes
    std::vector<pow_hash>     next_blocks;
};

FC_REFLECT( meta_block_header, (id)(error_message)(header)(next_blocks) )



