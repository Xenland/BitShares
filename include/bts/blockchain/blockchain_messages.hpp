#pragma once
#include <bts/blockchain/block.hpp>

namespace bts { namespace blockchain {

  enum message_type
  {
      trx_inv_msg         = 1,
      get_trx_inv_msg     = 2,
      block_inv_msg       = 3,
      get_block_inv_msg   = 4,
      get_trxs_msg        = 5,
      get_full_block_msg  = 6,
      get_trx_block_msg   = 7,
      trxs_msg            = 8,
      full_block_msg      = 9,
      trx_block_msg       = 10,
      message_type_count     /// used to verify message type range
  };

  struct trx_inv_message
  {
      static const message_type type = trx_inv_msg;
      std::vector<uint160> items;
  };

  struct block_inv_message
  {
      static const message_type type = block_inv_msg;
      /**
       * List of block headers starting with the
       * oldest block (in this sequence) and working forward to the
       * most recent block.  Contains the hashes of
       * blocks  [ChainLength-items.size, ChainLength] 
       */
      std::vector<fc::sha224> items;
  };

  struct get_trx_inv_message
  {
      static const message_type type = get_trx_inv_msg;
  };
  
  struct get_block_inv_message
  {
      static const message_type type = get_block_inv_msg;
      /** the block headers known by this node going back 
       * in powers of 2 for up to 1 year so that the
       * remote node can detect where our chains converge.
       */
      std::vector<fc::sha224> known;
  };

  struct get_trxs_message
  {
      static const message_type type = get_trxs_msg;
      get_trxs_message(){}

      get_trxs_message(const fc::sha224& one)
      :items(1,one){}

      std::vector<fc::sha224> items;
  };

  struct get_full_block_message
  {
      static const message_type type = get_full_block_msg;
      get_full_block_message( const fc::sha224& b )
      :block_id(b){}
      get_full_block_message(){}

      fc::sha224 block_id;
  };

  struct get_trx_block_message
  {
      static const message_type type = get_trx_block_msg;
      get_trx_block_message( const fc::sha224& b )
      :block_id(b){}
      get_trx_block_message(){}

      fc::sha224 block_id;
  };

  struct trxs_message
  {
     static const message_type type = trxs_msg;

     trxs_message(){}
     trxs_message(const signed_transaction& trx)
     :trxs(1,trx){}

     std::vector<signed_transaction> trxs;
  };

  struct full_block_message
  {
     static const message_type type = full_block_msg;

     full_block_message(){}
     full_block_message(const full_block& blk)
     :block_data(blk){}

     full_block block_data;
  };

  struct trx_block_message
  {
     static const message_type type = trx_block_msg;

     trx_block_message(){}
     trx_block_message(const trx_block& blk)
     :block_data(blk){}

     trx_block block_data;
  };


} } // bts::blockchain
FC_REFLECT_ENUM( bts::blockchain::message_type,
  (trx_inv_msg)
  (get_trx_inv_msg)
  (block_inv_msg)
  (get_block_inv_msg)
  (get_trxs_msg)
  (get_full_block_msg)
  (get_trx_block_msg)
  (trxs_msg)
  (full_block_msg)
  (trx_block_msg)
)

FC_REFLECT( bts::blockchain::trx_inv_message, (items) )
FC_REFLECT( bts::blockchain::block_inv_message, (items) )
FC_REFLECT_EMPTY( bts::blockchain::get_trx_inv_message )
FC_REFLECT( bts::blockchain::get_block_inv_message, (known) )
FC_REFLECT( bts::blockchain::get_trxs_message, (items) )
FC_REFLECT( bts::blockchain::get_full_block_message, (block_id) )
FC_REFLECT( bts::blockchain::get_trx_block_message, (block_id) )
FC_REFLECT( bts::blockchain::trxs_message, (trxs) )
FC_REFLECT( bts::blockchain::full_block_message, (block_data) )
FC_REFLECT( bts::blockchain::trx_block_message, (block_data) )

