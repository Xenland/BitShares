#include <bts/rpc/rpc_server.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/network/tcp_socket.hpp>
#include <fc/rpc/json_connection.hpp>
#include <fc/thread/thread.hpp>

namespace bts { namespace rpc { 

  namespace detail 
  {
    class server_impl
    {
       public:
         server::config      _config;
         bitname::client_ptr _bitnamec;

         fc::tcp_server      _tcp_serv;

         fc::future<void>    _accept_loop_complete;


         void accept_loop()
         {
           while( !_accept_loop_complete.canceled() )
           {
              fc::tcp_socket_ptr sock = std::make_shared<fc::tcp_socket>();
              _tcp_serv.accept( *sock );

              auto buf_istream = std::make_shared<fc::buffered_istream>( sock );
              auto buf_ostream = std::make_shared<fc::buffered_ostream>( sock );

              auto json_con = std::make_shared<fc::rpc::json_connection>( std::move(buf_istream), std::move(buf_ostream) );
              register_methods( json_con );

         //   TODO  0.5 BTC: handle connection errors and and connection closed without
         //   creating an entirely new context... this is waistful
         //     json_con->exec(); 
              fc::async( [json_con]{ json_con->exec().wait(); } );
         //     _connections.push_back( std::move( json_con ) );
           }
         }

         void register_methods( const fc::rpc::json_connection_ptr& con )
         {
            if( _bitnamec ) 
            {
               register_bitname_methods( con );
            }
         }

         void register_bitname_methods( const fc::rpc::json_connection_ptr& con )
         {
            con->add_method( "lookup_name", [=]( const fc::variants& v ) -> fc::variant 
            {
                FC_ASSERT( v.size() == 1 );
                return fc::variant( _bitnamec->lookup_name( v[0].as_string() ) );
            });
         }
    };
  } // detail

  server::server()
  :my( new detail::server_impl() )
  {
  }

  server::~server()
  { 
     try {
         my->_tcp_serv.close();
         if( my->_accept_loop_complete.valid() )
         {
            my->_accept_loop_complete.cancel();
            my->_accept_loop_complete.wait();
         }
     } 
     catch ( const fc::exception& e )
     {
        wlog( "unhandled exception thrown in destructor.\n${e}", ("e", e.to_detail_string() ) );
     }
  }

  void server::configure( const server::config& cfg )
  {
     try {
       my->_config = cfg;
       my->_tcp_serv.listen( cfg.port );
     // TODO shutdown server if already configured prior to restarting it
     
       my->_accept_loop_complete = fc::async( [=]{ my->accept_loop(); } );
     
     } FC_RETHROW_EXCEPTIONS( warn, "attempting to configure rpc server ${port}", ("port",cfg.port)("config",cfg) );
  }

  void server::set_bitname_client( const bts::bitname::client_ptr& bitnamec )
  {
     my->_bitnamec = bitnamec;
  }




} } // bts::rpc
