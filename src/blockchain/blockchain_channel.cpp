#include <bts/blockchain/blockchain_channel.hpp>
#include <bts/blockchain/blockchain_db.hpp>
#include <bts/blockchain/blockchain_messages.hpp>

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

#include <map>

namespace bts { namespace blockchain {

  using namespace network;
  namespace detail 
  {

     class chan_data : public network::channel_data
     {
        public:
          std::unordered_set<uint160>     known_trx_inv;
          std::unordered_set<fc::sha224>  known_block_inv;

          // fetches for which we have not yet received a reply...
          // TODO: add a timestamp so we can time it out properly....
          // Any connection that pushes data we didn't request is 
          // punished...
          std::unordered_set<uint160> requested_msgs; 
     };

     class channel_impl  : public bts::network::channel
     {
        public:
          bts::peer::peer_channel_ptr                      _peers;
          /* used to quickly filter out / prioritize trxs */
          std::map<fc::time_point, uint160>                _trx_time_index;

          /** validated transactions that are sent out with get inv msgs */
          std::unordered_map<uint160,signed_transaction>   _pending_trx;

          network::channel_id                              _chan_id; 
          blockchain_db_ptr                                _db;
          channel_delegate*                                _del;

      
          std::unordered_set<uint160>                      _trxs_pending_fetch;

          chan_data& get_channel_data( const connection_ptr& c )
          {
              auto cd = c->get_channel_data( _chan_id );
              if( !cd )
              {
                 cd = std::make_shared<chan_data>();
                 c->set_channel_data( _chan_id, cd );
              }
              chan_data& cdat = cd->as<chan_data>();
              return cdat;
          }


          virtual void handle_message( const connection_ptr& c, const bts::network::message& m )
          { 
            try { 
              chan_data& cdat = get_channel_data(c);

              ilog( "${msg_type}", ("msg_type", (blockchain::message_type)m.msg_type ) );
              switch( (blockchain::message_type)m.msg_type )
              {
                  case trx_inv_msg:
                     handle_trx_inv( c, cdat, m.as<trx_inv_message>() );
                     break;

                  case get_trx_inv_msg:
                     handle_get_trx_inv( c, cdat, m.as<get_trx_inv_message>() );
                     break;

                  case block_inv_msg:
                     handle_block_inv( c, cdat, m.as<block_inv_message>() );
                     break;

                  case get_block_inv_msg:
                     handle_get_block_inv( c, cdat, m.as<get_block_inv_message>() );
                     break;

                  case get_trxs_msg:
                     handle_get_trxs( c, cdat, m.as<get_trxs_message>() );
                     break;

                  case get_full_block_msg:
                     handle_get_full_block( c, cdat, m.as<get_full_block_message>() );
                     break;

                  case get_trx_block_msg:
                     handle_get_trx_block( c, cdat, m.as<get_trx_block_message>() );
                     break;

                  case trxs_msg:
                      handle_trxs( c, cdat, m.as<trxs_message>() );
                      break;

                  case full_block_msg:
                      handle_full_block( c, cdat, m.as<full_block_message>() );
                      break;

                  case trx_block_msg:
                      handle_trx_block( c, cdat, m.as<trx_block_message>() );
                      break;

                  default:
                     // TODO: figure out how to document this / punish the connection that sent us this 
                     // message.
                     wlog( "unknown blockchain message type ${t}", ("t",uint64_t(m.msg_type)) );
              }
            } 
            catch ( const fc::exception& e )
            {
               wlog( "exception thrown while handling message\n${e}", ("e", e.to_detail_string() ) );
               // TODO: punish the connection in some manner
            }
          } 

          /**
           *   @note inv_msg is passed by value so move semantics can be used.
           */
          void handle_trx_inv( const connection_ptr& c, chan_data& cdat, trx_inv_message msg )
          { try {
              for( auto itr = msg.items.begin(); itr != msg.items.end(); ++itr )
              {
                 if( !cdat.known_trx_inv.insert( *itr ).second )
                 {
                    wlog( "received inventory item more than once from the same connection\n",
                              ("item", *itr) );
                    // TODO: why is this connection sending things multiple times... punish it
                 }
                 _trxs_pending_fetch.insert( *itr );
              }
          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_get_trx_inv( const connection_ptr& c, chan_data& cdat, get_trx_inv_message msg )
          { try {
             // TODO: only allow this request once every couple of minutes to prevent flood attacks
             
             // TODO: make sure these inventory items are sorted by fees
             trx_inv_message reply;
             reply.items.reserve( 2000 ); // TODO define constant in config.hpp
             for( auto itr = _pending_trx.begin(); reply.items.size() < 2000 && itr != _pending_trx.end(); ++itr )
             {
                reply.items.push_back( itr->first );
             }
             c->send( network::message( reply, _chan_id ) );
          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_block_inv( const connection_ptr& c, chan_data& cdat, block_inv_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_get_block_inv( const connection_ptr& c, chan_data& cdat, get_block_inv_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_get_trxs( const connection_ptr& c, chan_data& cdat, get_trxs_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_get_full_block( const connection_ptr& c, chan_data& cdat, get_full_block_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_get_trx_block( const connection_ptr& c, chan_data& cdat, get_trx_block_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_trxs( const connection_ptr& c, chan_data& cdat, trxs_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_full_block( const connection_ptr& c, chan_data& cdat, full_block_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors

          void handle_trx_block( const connection_ptr& c, chan_data& cdat, trx_block_message msg )
          { try {

          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg) ) } // provide stack trace for errors
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
