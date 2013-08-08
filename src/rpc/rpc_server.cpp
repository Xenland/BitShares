#include <bts/rpc/rpc_server.hpp>
namespace bts { namespace rpc { 

  namespace detail 
  {
    class server_impl
    {
       public:
         server::config      _config;
         bitname::client_ptr _bitnamec;
    };
  } // detail

  server::server()
  :my( new detail::server_impl() )
  {
  }

  server::~server()
  { }

  void server::configure( const config& cfg )
  {
     my->_config = cfg;
     // TODO shutdown server if already configured prior to restarting it
     

  }

  void server::set_bitname_client( const bts::bitname::client_ptr& bitnamec )
  {
     my->_bitnamec = bitnamec;

     // register bitname methods... 
  }




} } // bts::rpc
