#include <bts/peer/peer_messages.hpp>
#include <bts/peer/peer_channel.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <unordered_map>
#include <algorithm>

namespace bts { namespace peer {
   using namespace bts::network;

   namespace detail 
   {

      class peer_data : public channel_data
      {
         public:
           peer_data():requested_hosts(false){}
           // data stored with the connection
           std::unordered_set<uint32_t> subscribed_channels;
           fc::ip::endpoint             public_contact; // used for reverse connection
           fc::optional<config_msg>     peer_config;
           bool                         requested_hosts;
      };

      class channel_connection_index
      {
         public:
             void add_connection( connection* c )
             {
                connections.push_back(c);
             }
             void remove_connection( connection* c )
             {
                auto p = std::find( connections.begin(), connections.end(), c );
                assert( p != connections.end() ); // invariant, crash in debug
                if( p != connections.end() ) // don't crash in release
                {
                   connections.erase(p);
                }
                else
                {
                   wlog("invariant not maintained" );
                }
             }
         private:
             std::vector<connection*> connections;
      };


      class peer_channel_impl : public channel, public server_delegate
      {
         public:
           server_ptr netw;
           
           /** maps a channel ID to all connections subscribed to that channel */
           std::unordered_map<uint32_t,channel_connection_index>  cons_by_channel;
           std::unordered_set<uint32_t>                           subscribed_channels;

           /**
            *  Store all hosts we know about sorted by time since we last heard about them.
            *  This list is provided to new nodes when they connect.  Limit to 1000 nodes.
            */
           std::vector<host>                                      recent_hosts;


           /**
            *  Stores the host in the known host list if it has a valid IP address and
            *  a recent time stamp.
            */
           bool store_host( const host& h )
           {
              if( !h.ep.get_address().is_public_address() )
              {
                return false;
              }

              fc::time_point expire_time = fc::time_point::now() - fc::seconds(60*60*3);

              if(  h.last_com < expire_time )
              {
                return false; // too old
              }

              /** remove any expired hosts while we are at it */
              for( auto itr = recent_hosts.begin(); itr != recent_hosts.end();  )
              {
                  if( itr->ep == h.ep )
                  {
                      if( itr->last_com < h.last_com )
                      {
                         itr->last_com = h.last_com;
                      }
                      return false; 
                  }
                  if( itr->last_com < expire_time )
                  {
                    itr = recent_hosts.erase(itr);      
                  }
                  else
                  {
                    ++itr;
                  }
              }
              if( recent_hosts.size() >= 1000 ) return false;
              recent_hosts.push_back(h);
              return true;
           }
           
           virtual void on_connected( const connection_ptr& c )
           {
               c->set_channel_data( channel_id( peer_proto ), std::make_shared<peer_data>() );
           }
           
           virtual void on_disconnected( const connection_ptr& c )
           {
               peer_data& pd = c->get_channel_data( channel_id(peer_proto) )->as<peer_data>(); 
               for( auto itr = pd.subscribed_channels.begin(); itr != pd.subscribed_channels.end(); ++itr )
               {
                  cons_by_channel[*itr].remove_connection(c.get());
               }
           }
           
           virtual void handle_message( const connection_ptr& c, const message& m )
           {
               if( m.msg_type == subscribe_msg::type )
               {
                   handle_subscribe( c, m.as<subscribe_msg>() );
               }
               else if( m.msg_type == unsubscribe_msg::type )
               {
                   handle_unsubscribe( c, m.as<unsubscribe_msg>() );
               }
               else if( m.msg_type == known_hosts_msg::type )
               {
                   handle_known_hosts( c, m.as<known_hosts_msg>() );
               }
               else if( m.msg_type == get_known_hosts_msg::type )
               {
                   handle_get_known_hosts( c, m.as<get_known_hosts_msg>() );
               }
               else if( m.msg_type == config_msg::type )
               {
                   handle_config( c, m.as<config_msg>() );
               }
               else if( m.msg_type == error_report_msg::type )
               {
                   handle_error_report( c, m.as<error_report_msg>() );
               }
           }

           void handle_config( const connection_ptr& c, config_msg cfg  )
           {
               peer_data& pd = c->get_channel_data( channel_id(peer_proto) )->as<peer_data>(); 
               pd.peer_config = std::move(cfg);

               if( recent_hosts.size() < PEER_HOST_CACHE_QUERY_LIMIT )
               {
                  pd.requested_hosts = true;
                  c->send( message( get_known_hosts_msg(), channel_id(peer_proto) ) );
               }
           }

           void handle_known_hosts( const connection_ptr& c, const known_hosts_msg& m )
           {
               peer_data& pd = c->get_channel_data( channel_id(peer_proto) )->as<peer_data>(); 

               std::vector<host> new_hosts;
               for( auto itr = m.hosts.begin(); itr != m.hosts.end(); ++itr )
               {
                  if( store_host( *itr ) && pd.requested_hosts )   
                  {
                    new_hosts.push_back(*itr);
                  }
               }
               pd.requested_hosts = false;

               if( new_hosts.size() )
               {
                  netw->broadcast( message( known_hosts_msg( new_hosts ) , channel_id( peer_proto) ) );
               }
           }
           
           void handle_subscribe( const connection_ptr& c, const subscribe_msg& s )
           {
               peer_data& pd = c->get_channel_data( channel_id(peer_proto) )->as<peer_data>(); 
               if( s.channels.size() > MAX_CHANNELS_PER_CONNECTION )
               {
                  // TODO... send a error message... disconnect host... require pow?
               }
           
               for( auto itr = s.channels.begin(); itr != s.channels.end(); ++itr )
               {
                   if( pd.subscribed_channels.insert( itr->id() ).second )
                   {
                      // TODO: validate ID is an acceptable / supported channel to prevent
                      // remote hosts from sending us a ton of bogus channels
                      cons_by_channel[itr->id()].add_connection(c.get());
                   }
               }
           }
           
           void handle_unsubscribe( const connection_ptr& c, const unsubscribe_msg& s )
           {
               peer_data& pd = c->get_channel_data( channel_id(peer_proto) )->as<peer_data>(); 
               for( auto itr = s.channels.begin(); itr != s.channels.end(); ++itr )
               {
                   if( pd.subscribed_channels.erase( itr->id() ) != 0 )
                   {
                      // TODO: validate ID is an acceptable / supported channel to prevent
                      // remote hosts from sending us a ton of bogus channels
                      cons_by_channel[itr->id()].remove_connection(c.get());
                   }
               }
           }

           void handle_get_subscribed( const connection_ptr& c, const get_subscribed_msg& m ) 
           {
              // TODO: rate control this query???
              subscribe_msg s;
              for( auto i = subscribed_channels.begin(); i != subscribed_channels.end(); ++i)
              {
                s.channels.push_back(channel_id(*i));
              }
              c->send( message( s, channel_id( peer_proto ) ) );
           }

           void handle_get_known_hosts( const connection_ptr& c, const get_known_hosts_msg& h )
           {
              c->send( message( known_hosts_msg( recent_hosts), channel_id(peer_proto) ) );
           }

           void handle_error_report( const connection_ptr& c, const error_report_msg& m)
           {
              wlog( "${reprot}", ("reprot",m) );
           }
      };




   }

   peer_channel::peer_channel( const server_ptr& s )
   :my( new detail::peer_channel_impl() )
   {
      my->netw = s;
      s->set_delegate( my.get() );
   }

   peer_channel::~peer_channel()
   {
      my->netw->unsubscribe_from_channel( channel_id(peer_proto) );
   }

   void peer_channel::subscribe_to_channel( const channel_id& chan, const channel_ptr& c )
   {
      // let the network know to forward messages to c
      my->netw->subscribe_to_channel( chan, c );

      // let other peers know that we are now subscribed to chan
      subscribe_msg s;
      s.channels.push_back(chan);
      my->netw->broadcast( message( s, channel_id( peer_proto ) ) );
   }

   void peer_channel::unsubscribe_from_channel( const channel_id& chan )
   {
      my->netw->unsubscribe_from_channel(chan);

      // let other peers know that we are now unsubscribed from chan
      unsubscribe_msg u;
      u.channels.push_back(chan);
      my->netw->broadcast( message( u, channel_id( peer_proto ) ) );
   }




} } // bts::network
