#pragma once
#include "blockchain.hpp"
#include <fc/filesystem.hpp>

namespace detail { class database_impl; }

struct meta_block
{
    block_header                  head;
    std::vector<fc::sha224>       trxs;
};
FC_REFLECT( meta_block, (head)(trxs) )

typedef std::shared_ptr<meta_block> meta_block_ptr;

struct meta_signed_transaction
{
    pow_hash             blk; // block in longest chain that contains this trx
    signed_transaction   trx;  
};
FC_REFLECT( meta_signed_transaction, (blk)(trx) )

typedef std::shared_ptr<meta_signed_transaction> meta_signed_transaction_ptr;

struct meta_output
{
   meta_output( const trx_output& o, const fc::sha224& i )
   :output(o),trx_id(i){}
   meta_output(){}

   trx_output   output;
   fc::sha224   trx_id;    // the trx that included this output
};
FC_REFLECT( meta_output, (output)(trx_id)(block_num)(spent) )

typedef std::shared_ptr<meta_output> meta_output_ptr;

/**
 *  Provides access to all known objects and their state independent of
 *  their inclusion in any particular block chain or 'spent' status.  This
 *  can be thought of as the 'raw' building blocks of the blockchain.
 *
 *  Tables:
 *    Blocks along with meta info about next/prev
 *    Transactions indexed by trx_id along with meta-info about blocks that contain it
 *    Outputs indexed by output_id => trx_id + output idx + output data
 *    Address -> outputs that can spend it
 */
class database
{
  public:
     database();
     ~database();

     void load( const fc::path& db_dir );
     void close();

     /**
      *  Store the block in the database along with all included transactions
      */
     void                         store( const block& b, const pow_hash& block_id );

     /**
      *   Store the trx in the database and note which blocks it was referenced by.
      */
     meta_signed_transaction_ptr  store( const signed_transaction& trx, 
                                         const pow_hash& block_id  = pow_hash() );

     meta_block_ptr               fetch_block( const pow_hash& block_id );
     meta_signed_transaction_ptr  fetch_transaction( const fc::sha224& trx_id );
     meta_output_ptr              fetch_output( const fc::sha224& output_id );

     /**
      *  @return all outputs 'spendable' by adr
      */
     std::vector<meta_output_ptr>       fetch_outputs_by_address( const address& adr );

  private:
     std::unique_ptr<detail::database_impl> my;

};
