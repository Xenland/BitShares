#pragma once
#include <bts/bitname/name_client.hpp>

namespace bts { namespace rpc { 

  namespace detail { class server_impl; }

  /**
   *  @brief provides a JSON-RPC interface bitshare components
   *
   *  Before any RPC commands will be processed, the user must call
   *  the login method and get a valid response. 
   *
   *  @code
   *   { "method":"login", "params":["user","pass"], "id":0 }
   *  @endcode
   *
   *  TODO: Implement SSL connections, perhaps with CERT requirement
   *  TODO: Restrict connections to a user-specified interface (ie: loopback)
   */
  class server
  {
    public:
      struct config 
      {
         config()
         :port(0){}

         uint16_t      port;
         std::string   user;
         std::string   pass;
      };

      server();
      ~server();

      void configure( const config& cfg );
      void set_bitname_client( const bts::bitname::client_ptr& name_cl );

    private:
      std::unique_ptr<detail::server_impl> my;
  };

  typedef std::shared_ptr<server> server_ptr;

} } // bts::rpc

FC_REFLECT( bts::rpc::server::config, (port)(user)(pass) )
