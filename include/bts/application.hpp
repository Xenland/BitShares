#pragma once
#include <bts/config.hpp>
#include <bts/profile.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>
#include <bts/bitname/bitname_client.hpp>

namespace bts {

  namespace detail { class application_impl; }

  struct application_config
  {
      application_config()
      :network_port(NETWORK_DEFAULT_PORT),
       rpc_port(RPC_DEFAULT_PORT){}

      fc::path  data_dir;
      uint16_t  network_port;
      uint16_t  rpc_port;
  };

  class application_delegate
  {
     public:

     virtual ~application_delegate(){}

     virtual void received_text( const bitchat::private_text_message& msg, 
                                 const fc::ecc::public_key& from, 
                                 const fc::ecc::public_key& to ){}

     virtual void received_email( const bitchat::private_email_message& msg, 
                                  const fc::ecc::public_key& from, 
                                  const fc::ecc::public_key& to ){}
  };
  

  /**
   *  This class serves as the interface between the GUI and the back end
   *  business logic.  All external interfaces (RPC, Web, Qt, etc) should
   *  interact with this API and not access lower-level apis directly.  
   */
  class application
  {
    public:
      application();
      ~application();

      void                        configure( const application_config& cfg );
      application_config          get_configuration()const;

      void                        set_application_delegate( application_delegate* del );
                                  
      profile_ptr                 get_profile();
      profile_ptr                 load_profile( const std::string& password );
      profile_ptr                 create_profile( const profile_config& cfg, const std::string& password );
                                  
      fc::optional<bitname::name_record>   lookup_name( const std::string& name );
      fc::optional<bitname::name_record>   reverse_name_lookup( const fc::ecc::public_key& key );
      void                                 mine_name( const std::string& name, const fc::ecc::public_key& key, float effort = 0.1 );

      void  send_contact_request( const fc::ecc::public_key& to, const fc::ecc::private_key& from );
      void  send_email( const bitchat::private_email_message& email, const fc::ecc::public_key& to, const fc::ecc::private_key& from );
      void  send_text_message( const bitchat::private_text_message& txtmsg, const fc::ecc::public_key& to, const fc::ecc::private_key& from );

    private:
      std::unique_ptr<detail::application_impl> my;
  };

} // namespace bts

FC_REFLECT( bts::application_config, (data_dir)(network_port)(rpc_port) )
