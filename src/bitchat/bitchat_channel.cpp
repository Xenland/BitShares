#include <bts/bitchat/bitchat_channel.hpp>
#include <bts/bitchat/bitchat_messages.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>
#include <unordered_map>
#include <map>

namespace bts { namespace bitchat {

  using network::channel_id;
  using network::connection_ptr;

  namespace detail 
  {
     class bitchat_chan_data : public network::channel_data
     {
        public:
          std::unordered_set<mini_pow> known_inv;
     };


     class channel_impl : public bts::network::channel 
     {
        public:
          channel_id               chan_id;
          channel_delegate*        del;
          peer::peer_channel_ptr   peers;

          std::map<fc::time_point, mini_pow>            msg_time_index;
          std::unordered_map<mini_pow,encrypted_message>  priv_msgs;

          /// messages that we have recieved inv for, but have not requested the data for
          std::unordered_set<mini_pow>                  unknown_msgs; 
          std::unordered_map<mini_pow,fc::time_point>   requested_msgs; // messages that we have requested but not yet received

          std::vector<mini_pow>                         new_msgs;  // messages received since last inv broadcast

          fc::future<void>                              fetch_loop_complete;

          /**
           *  Get or create the bitchat channel data for this connection and return
           *  a reference to the result.
           */
          bitchat_chan_data& get_channel_data( const connection_ptr& c )
          {
              auto cd = c->get_channel_data( chan_id );
              if( !cd )
              {
                 cd = std::make_shared<bitchat_chan_data>();
                 c->set_channel_data( chan_id, cd );
              }
              bitchat_chan_data& cdat = cd->as<bitchat_chan_data>();
              return cdat;
          }

          virtual void handle_message( const connection_ptr& c, const bts::network::message& m )
          {
              bitchat_chan_data& cdat = get_channel_data(c);

              ilog( "${msg_type}", ("msg_type", (bitchat::message_type)m.msg_type ) );
              switch( (bitchat::message_type)m.msg_type )
              {
                  case inv_msg:
                     handle_inv( c, cdat, m.as<inv_message>()  );
                     break;
                  case get_inv_msg:
                     handle_get_inv( c, cdat, m.as<get_inv_message>()  );
                     break;
                  case get_priv_msg:
                     handle_get_priv( c, cdat, m.as<get_priv_message>()  );
                     break;
                  case encrypted_msg:
                     handle_priv_msg( c, cdat, m.as<encrypted_message>()  );
                     break;
                  default:
                     // TODO: figure out how to document this / punish the connection that sent us this 
                     // message.
                     wlog( "unknown bitchat message type ${t}", ("t",uint64_t(m.msg_type)) );
              }
          }

          void fetch_loop()
          {
             try {
                while( !fetch_loop_complete.canceled() )
                {
                   broadcast_inv();
                   if( unknown_msgs.size()  )
                   {
                      auto cons = peers->get_connections( chan_id );
                      // copy so we don't hold shared state in the iterators
                      // while we iterate and send fetch requests
                      auto tmp = unknown_msgs; 
                      for( auto itr = tmp.begin(); itr != tmp.end(); ++itr )
                      {
                          fetch_from_best_connection( cons, *itr );
                      }
                   }
                   /* By using a random sleep we give other peers the oppotunity to find
                    * out about messages before we pick who to fetch from.
                    */
                   fc::usleep( fc::microseconds( (rand() % 20000) + 100) ); // note: usleep(0) sleeps forever... perhaps a bug?
                }
             } 
             catch ( ... )
             {
               wlog( "TODO: handle exceptions here???" );
             }
          }

          /**
           *   For any given message id, there are many potential hosts from which it could be fetched.  We
           *   want to distribute the load across all hosts equally and therefore, the best one to fetch from
           *   is the host that we have fetched the least from and that has fetched the most from us.
           *
           */
          void fetch_from_best_connection( const std::vector<connection_ptr>& cons, const mini_pow& id )
          {
             // if request is made, move id from unknown_msgs to requested_msgs 
             // TODO: update this algorithm to be something better. 
             for( uint32_t i = 0; i < cons.size(); ++i )
             {
                 bitchat_chan_data& cd = get_channel_data(cons[i]); 
                 if( cd.known_inv.find( id ) !=  cd.known_inv.end() )
                 {
                    requested_msgs[id] = fc::time_point::now();
                    unknown_msgs.erase(id);
                    cons[i]->send( network::message( get_priv_message( id ), chan_id ) );
                    return;
                 }
             }
          }

          /**
           *  Send any new inventory items that we have received since the last
           *  broadcast to all connections that do not know about the inv item.
           */
          void broadcast_inv()
          {
              if( new_msgs.size() )
              {
                auto cons = peers->get_connections( chan_id );
                for( auto c = cons.begin(); c != cons.end(); ++c )
                {
                  inv_message msg;

                  bitchat_chan_data& cd = get_channel_data( *c );
                  for( uint32_t i = 0; i < new_msgs.size(); ++i )
                  {
                     if( cd.known_inv.insert( new_msgs[i] ).second )
                     {
                        msg.items.push_back( new_msgs[i] );
                     }
                  }

                  if( msg.items.size() )
                  {
                    (*c)->send( network::message(msg,chan_id) );
                  }
                }
                new_msgs.clear();
              }
          }


          /**
           *  Note that c knows about items and add any unknown items to our queue 
           */
          void handle_inv( const connection_ptr& c, bitchat_chan_data& cd, const inv_message& msg )
          {
              ilog( "inv: ${msg}", ("msg",msg) );
              for( auto itr = msg.items.begin(); itr != msg.items.end(); ++itr )
              {
                 cd.known_inv.insert( *itr );
                 if( priv_msgs.find( *itr ) == priv_msgs.end() )
                 {
                    unknown_msgs.insert( *itr );
                 }
              }
          }

          /**
           *  Send all inventory items that are not known to c and are dated 'after'
           */
          void handle_get_inv( const connection_ptr& c, bitchat_chan_data& cd, const get_inv_message& msg )
          {
             inv_message reply;
             for( auto itr = msg_time_index.lower_bound( fc::time_point(msg.after) ); itr != msg_time_index.end(); ++itr )
             {
                if( cd.known_inv.insert( itr->second ).second )
                {
                   reply.items.push_back( itr->second );
                   cd.known_inv.insert( itr->second );
                }
             }
             c->send( network::message( reply, chan_id ) );
          }
            

          void handle_get_priv( const connection_ptr& c, bitchat_chan_data& cd, const get_priv_message& msg )
          {
             // TODO: throttle the rate at which get_priv may be called for a given connection
             for( auto itr = msg.items.begin(); itr != msg.items.end(); ++itr )
             {
                auto m = priv_msgs.find( *itr );
                if( m != priv_msgs.end() )
                {
                   c->send( network::message( m->second, chan_id ) );
                   cd.known_inv.insert( *itr ); 
                }
             }
          }

          void handle_priv_msg( const connection_ptr& c, bitchat_chan_data& cd, encrypted_message&& msg )
          {
              auto mid = msg.id();

              // validate proof of work 
              // validate timestamp
              // store in index
              if( priv_msgs.find(mid) == priv_msgs.end() )
              {
                 new_msgs.push_back( mid );
                 msg_time_index[fc::time_point::now()] = mid;
                 const encrypted_message& m = (priv_msgs[mid] = std::move(msg));
                 del->handle_message( m, chan_id );
              }
              else
              {
                 wlog( "duplicate message received" );
              }
              // schedule to broadcast to peers on channel that haven't told us about it yet
          }
     };


  } // namespace detail
  
  channel::channel( const bts::peer::peer_channel_ptr& p, const channel_id& c, channel_delegate* d )
  :my( std::make_shared<detail::channel_impl>() )
  {
     assert( d != nullptr );
     my->peers = p;
     my->del = d;
     my->chan_id = c;

     my->peers->subscribe_to_channel( c, my );

     my->fetch_loop_complete = fc::async( [=](){ my->fetch_loop(); } );
  }

  channel::~channel()
  {
     my->peers->unsubscribe_from_channel( my->chan_id );
     my->del = nullptr;
     try {
        my->fetch_loop_complete.cancel();
        my->fetch_loop_complete.wait();
     } 
     catch ( ... ) 
     {
        wlog( "unexpected exception ${e}", ("e", fc::except_str()));
     }
  }


  channel_id channel::get_id()const { return my->chan_id; }

  /**
   *  Places the message into the queue of received messages as if
   *  we had received it from another node.  The next time my->broadcast_inv()
   *  is called it will include this message in the inventory list and
   *  it will appear indistigusable to other nodes.
   */
  void channel::broadcast( encrypted_message&& m )
  {
      //TODO: make 30 a constant in bts/config.hpp
      FC_ASSERT( fc::time_point::now() - m.timestamp  <  fc::seconds(30) );
      FC_ASSERT( m.timestamp <= fc::time_point::now() );

      auto id = m.id();
      my->priv_msgs[ id ] = std::move(m);
      my->msg_time_index[ m.timestamp ] = id;
      my->new_msgs.push_back(id);
  }


} } // namespace bts::bitchat
