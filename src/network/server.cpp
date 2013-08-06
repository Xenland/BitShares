#include <bts/network/server.hpp>
#include <bts/network/connection.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/thread.hpp>
#include <fc/thread/future.hpp>
#include <fc/io/raw.hpp>
#include <fc/log/logger.hpp>


#include <algorithm>
#include <unordered_map>
#include <map>

namespace bts { namespace network {

  namespace detail
  {
     class server_impl : public connection_delegate
     {
        public:
          server_impl()
          :ser_del(nullptr)
          {}

          ~server_impl()
          {
             close();
          }
          void close()
          {
              try 
              {
                  for( auto i = pending_connections.begin(); i != pending_connections.end(); ++i )
                  {
                    (*i)->close();
                  }
                  tcp_serv.close();
                  if( accept_loop_complete.valid() )
                  {
                      accept_loop_complete.wait();
                  }
              } 
              catch ( const fc::canceled_exception& e )
              {
                  ilog( "expected exception on closing tcp server\n" );  
              }
              catch ( const fc::exception& e )
              {
                  wlog( "unhandled exception in destructor ${e}", ("e", e.to_detail_string() ));
              } 
              catch ( ... )
              {
                  elog( "unexpected exception" );
              }
          }
          server_delegate*                                            ser_del;

          std::unordered_map<fc::ip::endpoint,connection_ptr>         connections;

          std::set<connection_ptr>                                    pending_connections;
          server::config                                              cfg;
          fc::tcp_server                                              tcp_serv;
                                                                     
          fc::future<void>                                            accept_loop_complete;
                                                                     
          std::unordered_map<uint32_t, channel_ptr>                   channels;

          virtual void on_connection_message( connection& c, const message& m )
          {
             auto itr = channels.find( m.channel().id() );
             if( itr != channels.end() )
             {
                // TODO: perhaps do this ASYNC?
                itr->second->handle_message( c.shared_from_this(), m );
             }
             else
             {
                wlog( "received message from unknown channel ${c} ", ("c", m.channel()) ); 
             }
          }

          virtual void on_connection_disconnected( connection& c )
          {
            try {
              ilog( "cleaning up connection after disconnect ${e}", ("e", c.remote_endpoint()) );
              auto cptr = c.shared_from_this();
              FC_ASSERT( ser_del != nullptr );
              FC_ASSERT( cptr );
              ser_del->on_disconnected( cptr );
              connections.erase( c.remote_endpoint() );
            } FC_RETHROW_EXCEPTIONS( warn, "error thrown handling disconnect" );
          }

          /**
           *  This method is called via async from accept_loop and
           *  should not throw any exceptions because they are not
           *  being caught anywhere.
           *
           *  
           */
          void accept_connection( const stcp_socket_ptr& s )
          {
             try 
             {
                // init DH handshake, TODO: this could yield.. what happens if we exit here before
                // adding s to connections list.
                s->accept();
                ilog( "accepted connection from ${ep}", 
                      ("ep", std::string(s->get_socket().remote_endpoint()) ) );
                
                auto con = std::make_shared<connection>(s,this);
                connections[con->remote_endpoint()] = con;
                ser_del->on_connected( con );
             } 
             catch ( const fc::canceled_exception& e )
             {
                ilog( "canceled accept operation" );
             }
             catch ( const fc::exception& e )
             {
                wlog( "error accepting connection: ${e}", ("e", e.to_detail_string() ) );
             }
             catch( ... )
             {
                elog( "unexpected exception" );
             }
          }

          /**
           *  This method is called async 
           */
          void accept_loop() throw()
          {
             try
             {
                stcp_socket_ptr sock = std::make_shared<stcp_socket>();
                while( tcp_serv.accept( sock->get_socket() ) )
                {
                   // do the acceptance process async
                   fc::async( [=](){ accept_connection( sock ); } );

                   // limit the rate at which we accept connections to prevent
                   // DOS attacks.
                   fc::usleep( fc::microseconds( 1000*10 ) );
                   sock = std::make_shared<stcp_socket>();
                }
             } 
             catch ( fc::eof_exception& e )
             {
                ilog( "accept loop eof" );
             }
             catch ( fc::canceled_exception& e )
             {
                ilog( "accept loop canceled" );
             }
             catch ( fc::exception& e )
             {
                elog( "tcp server socket threw exception\n ${e}", 
                                     ("e", e.to_detail_string() ) );
                // TODO: notify the server delegate of the error.
             }
             catch( ... )
             {
                elog( "unexpected exception" );
             }
          }
     };
  }




  server::server()
  :my( new detail::server_impl() ){}

  server::~server()
  { }

  void server::subscribe_to_channel( const channel_id& chan, const channel_ptr& c )
  {
     FC_ASSERT( my->channels.find(chan.id()) == my->channels.end() );
     my->channels[chan.id()] = c;
  }

  void server::unsubscribe_from_channel( const channel_id&  chan )
  {
     my->channels.erase(chan.id());
  }

  void server::set_delegate( server_delegate* sd )
  {
     my->ser_del = sd;
  }

  void server::configure( const server::config& c )
  {
    try {
      my->cfg = c;

      ilog( "listening for stcp connections on port ${p}", ("p",c.port) );
      my->tcp_serv.listen( c.port );
      my->accept_loop_complete = fc::async( [=](){ my->accept_loop(); } ); 
    } FC_RETHROW_EXCEPTIONS( warn, "error configuring server", ("config", c) );
  }



  std::vector<connection_ptr> server::get_connections()const
  { 
      std::vector<connection_ptr>  cons; 
      cons.reserve( my->connections.size() );
      for( auto itr = my->connections.begin(); itr != my->connections.end(); ++itr )
      {
        cons.push_back(itr->second);
      }
      return cons;
  }
  void server::broadcast( const message& m )
  {
      for( auto itr = my->connections.begin(); itr != my->connections.end(); ++itr )
      {
        try {
           itr->second->send(m);
        } 
        catch ( const fc::exception& e ) 
        {
           // TODO: propagate this exception back via the delegate or some other means... don't just fail
           wlog( "exception thrown while broadcasting ${e}", ("e", e.to_detail_string() ) );
        }
      }
  }

  connection_ptr server::connect_to( const fc::ip::endpoint& ep )
  {
     try
     {
       ilog( "connect to ${ep}", ("ep",ep) );
       FC_ASSERT( my->ser_del != nullptr );
       connection_ptr con = std::make_shared<connection>( my.get() );
       con->connect(ep);
       my->connections[con->remote_endpoint()] = con;
       my->ser_del->on_connected( con );
       return con;
     } FC_RETHROW_EXCEPTIONS( warn, "unable to connect to ${ep}", ("ep",ep) );
  }


   void server::close()
   {
     try {
       my->close();
     } FC_RETHROW_EXCEPTIONS( warn, "error closing server socket" );
   }



} } // namespace bts::network
