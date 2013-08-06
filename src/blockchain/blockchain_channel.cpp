#include <bts/blockchain/blockchain_channel.hpp>
#include <bts/blockchain/blockchain_db.hpp>

namespace bts { namespace blockchain {

  using namespace network;
  namespace detail 
  {
     class channel_impl  : public bts::network::channel
     {
        public:
          bts::peer::peer_channel_ptr                      _peers;
          std::unordered_map<uint160,signed_transaction>   _pending_trx;
          network::channel_id                              _chan_id; 
          blockchain_db_ptr                                _db;
          channel_delegate*                                _del;

          virtual void handle_message( const connection_ptr& c, const bts::network::message& m )
          {

          }

      
     };

  } // namespace detail 

  channel::channel( const bts::peer::peer_channel_ptr& peers,
                    const blockchain_db_ptr& db,
                    channel_delegate* d,
                    const network::channel_id& c
                    )
  :my( std::make_shared<detail::channel_impl>() )
  {
     my->_peers   = peers;
     my->_chan_id = c;
     my->_db      = db;
     my->_del     = d;

     my->_peers->subscribe_to_channel( my->_chan_id, my );
  }

  channel::~channel()
  {
  }
  
  network::channel_id channel::get_id()const
  {
    return my->_chan_id;
  }
      
  /**
   *  All transactions that are known but that have not been included in 
   *  a block that has been added to the blockchain_db
   */
  const std::unordered_map<uint160,signed_transaction>& channel::get_pending_pool()const
  {
      return my->_pending_trx;
  }

       /**
        *  Called when this node wishes to pubish a trx.
        */
  void channel::broadcast( const signed_transaction& trx )
  {
  }

       /**
        *  This is called when a new block is sloved by this node.
        */
  void channel::broadcast( const trx_block& b )
  {
  }
        

} }  // namespace bts::blockchain
