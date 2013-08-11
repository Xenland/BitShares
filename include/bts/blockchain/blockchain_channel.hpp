#pragma once
#include <bts/blockchain/blockchain_db.hpp>
#include <bts/peer/peer_channel.hpp>

#include <unordered_map>

namespace bts { namespace blockchain {

  namespace detail { class channel_impl; }

  class channel_delegate
  {
     public:
        virtual ~channel_delegate(){}
        /**
         *  Every time a new transaction is received this method will be 
         *  called.  
         */
        virtual void handle_trx( const signed_transaction& trx ){};
        virtual void handle_block(  const block& b ){};
        virtual void handle_trx_block( const trx_block& b ){};
  };

  /**
   *  The blockchain channel receives transactions, checks them
   *  against the current transaction DB and forwards them if they
   *  are valid.  
   *
   *  It any transactions that are deemed invalid are marked as such
   *  and discarded.  Any double-spends are only forwarded if there
   *  are more fees, both trx are valid are are held until one or
   *  the other is made invalid by a block.
   *
   *  When a request for a trx_id comes in, the pending pool is checked
   *  first, then the trx_db.  Each connection is only allowed to make
   *  a limited number of queries from the trx_db before a proof of
   *  work is requested.  This proof of work will help this client
   *  mine and thus pay for the effort.
   *
   *  Every node sets a level of work required to download a block 
   *  propotional to the cost of their bandwidth.  
   */
  class channel 
  {
     public:
       channel( const bts::peer::peer_channel_ptr& peers, 
                const blockchain_db_ptr& db,
                channel_delegate* d,
                const network::channel_id& c = network::channel_id( network::bts_proto, 0 )
                );

       ~channel();
  
       network::channel_id get_id()const;
      
       /**
        *  All transactions that are known but that have not been included in 
        *  a block that has been added to the blockchain_db
        */
       const std::unordered_map<uint160,signed_transaction>& get_pending_pool()const;

       /**
        *  Called when this node wishes to pubish a trx.
        */
       void broadcast( const signed_transaction& trx );

       /**
        *  This is called when a new block is sloved by this node.
        */
       void broadcast( const trx_block& b );

     private:
       std::shared_ptr<detail::channel_impl> my;
  };

  typedef std::shared_ptr<channel> channel_ptr;

} } // bts::blockchain
