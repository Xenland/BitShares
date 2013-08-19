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

         /** the set of connections that have successfully logged in */
         std::unordered_set<fc::rpc::json_connection*> _login_set;


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
            // don't capture the shared ptr, it would create a circular reference
            fc::rpc::json_connection* capture_con = con.get(); 
            con->add_method( "login", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 2 );
                FC_ASSERT( params[0].as_string() == _config.user )
                FC_ASSERT( params[1].as_string() == _config.pass )
                _login_set.insert( capture_con );
                return fc::variant( true );
            });
            if( _bitnamec ) 
            {
               register_bitname_methods( con );
            }
         }

         void check_login( fc::rpc::json_connection* con )
         {
            if( _login_set.find( con ) == _login_set.end() )
            {
               FC_THROW_EXCEPTION( exception, "not logged in" ); 
            }
         }

         void register_bitname_methods( const fc::rpc::json_connection_ptr& con )
         {
            // don't capture the shared ptr, it would create a circular reference
            fc::rpc::json_connection* capture_con = con.get(); 
            con->add_method( "lookup_name", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 1 );
                check_login( capture_con );
                return fc::variant( _bitnamec->lookup_name( params[0].as_string() ) );
            });
            con->add_method( "reverse_name_lookup", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 1 );
                check_login( capture_con );
                return fc::variant( _bitnamec->reverse_name_lookup( params[0].as<fc::ecc::public_key>() ) );
            });
            con->add_method( "verify_signature", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 2 );
                check_login( capture_con );
                return fc::variant( _bitnamec->verify_signature( params[0].as<fc::sha256>(), params[1].as<fc::ecc::compact_signature>() ) );
            });
            con->add_method( "sign", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 2 );
                check_login( capture_con );
                // TODO: this requires private key access not available to bitname... must come from 
                // profile..
                return fc::variant(/* _bitnamec->sign( params[0].as<fc::sha256>(), params[1].as_string() )*/ );
            });
            con->add_method( "register_name", [=]( const fc::variants& params ) -> fc::variant 
            {
                FC_ASSERT( params.size() == 2 );
                check_login( capture_con );
                _bitnamec->mine_name( params[0].as_string(), params[1].as<fc::ecc::public_key>() );
                return fc::variant();
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
     catch ( const fc::canceled_exception& e ){}
     catch ( const fc::exception& e )
     {
        wlog( "unhandled exception thrown in destructor.\n${e}", ("e", e.to_detail_string() ) );
     }
  }

  void server::configure( const server::config& cfg )
  {
     try {
       FC_ASSERT( cfg.port != 0 );
       my->_config = cfg;
       ilog( "listening for rpc connections on port ${port}", ("port",cfg.port) );
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
