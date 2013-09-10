#include <bts/application.hpp>
#include <bts/bitname/bitname_client.hpp>
#include <bts/bitchat/bitchat_client.hpp>
#include <bts/network/upnp.hpp>
#include <fc/reflect/variant.hpp>

#include <fc/log/logger.hpp>

namespace bts {

  namespace detail
  {
    class application_impl : public bts::bitname::client_delegate,
                             public bts::bitchat::client_delegate
    {
       public:
          application_impl()
          :_delegate(nullptr){}

          application_delegate*             _delegate;
          fc::optional<application_config>  _config;
          profile_ptr                       _profile;
          fc::path                          _profile_dir;

          bts::network::server_ptr          _server;
          bts::peer::peer_channel_ptr       _peers;
          bts::bitname::client_ptr          _bitname_client;
          bts::bitchat::client_ptr          _bitchat_client;     
          bts::network::upnp_service        _upnp;


          virtual void bitchat_message_received( const bitchat::decrypted_message& m )
          {
              ilog( "received ${msg}", ("msg",m) );
              if( _profile ) _profile->cache( m );
              switch( m.msg_type )
              {
                 case bitchat::private_message_type::text_msg:
                 {
                   auto txt = m.as<bitchat::private_text_message>();
                   ilog( "text message ${msg}", ("msg",txt) );
                   if( _delegate ) _delegate->received_text( txt, *m.from_key, m.decrypt_key->get_public_key() );
                   break;
                 }
                 case bitchat::private_message_type::email_msg:
                 {
                   auto email = m.as<bitchat::private_email_message>();
                   ilog( "email message ${msg}", ("msg",m.as<bitchat::private_email_message>()) );
                   if( _delegate ) _delegate->received_email( email, *m.from_key, m.decrypt_key->get_public_key() );
                   break;
                 }
              }
          }

          virtual void bitname_block_added( const bts::bitname::name_block& h )
          {
        //      ilog( "${h}", ("h",h) );
          }
         
          virtual void bitname_header_pending( const bts::bitname::name_header& h )
          {
         //     ilog( "${h}", ("h",h) );
              /*
              if( h.name_hash == current_name )
              {
                 current_name = fc::time_point::now().time_since_epoch().count();
          
                auto next_name = std::string(fc::time_point::now()); 
                ilog( "registering next name ${name}", ("name",next_name));
                 _client->mine_name( next_name,
                                         fc::ecc::private_key::generate().get_public_key() );
              }
              */
          }  
    };
  }

  application::application()
  :my( new detail::application_impl() )
  {
  }

  application::~application(){}

  void application::configure( const application_config& cfg )
  { try {
     my->_config = cfg;
     my->_profile_dir = cfg.data_dir / "profiles";
     
     fc::create_directories( my->_profile_dir );

     if( cfg.enable_upnp )
     {
        my->_upnp.map_port( cfg.network_port );
     }


     my->_server = std::make_shared<bts::network::server>();    

     bts::network::server::config server_cfg;
     server_cfg.port = cfg.network_port;

     my->_server->configure( server_cfg );

     my->_peers            = std::make_shared<bts::peer::peer_channel>(my->_server);
     my->_bitname_client   = std::make_shared<bts::bitname::client>( my->_peers );
     my->_bitname_client->set_delegate( my.get() );

     bitname::client::config bitname_config;
     bitname_config.data_dir = cfg.data_dir / "bitname";

     my->_bitname_client->configure( bitname_config );

     my->_bitchat_client  = std::make_shared<bts::bitchat::client>( my->_peers, my.get() );
     my->_bitchat_client->configure( cfg.data_dir / "bitchat" );

  } FC_RETHROW_EXCEPTIONS( warn, "", ("config",cfg) ) }

  application_config application::get_configuration()const
  {
     FC_ASSERT( my->_config );
     return *my->_config;
  }

  void      application::set_application_delegate( application_delegate* del )
  {
     my->_delegate = del;
  }

  bool                        application::has_profile()const
  {
     return fc::exists( my->_profile_dir / "default" );
  }

  profile_ptr                 application::get_profile()
  {
    FC_ASSERT( my->_config );
    return my->_profile;
  }

  profile_ptr                 application::load_profile( const std::string& password )
  { try {
    FC_ASSERT( my->_config );

    // note: stored in temp incase open throws.
    auto tmp_profile = std::make_shared<profile>();
    tmp_profile->open( my->_profile_dir / "default", password );

    std::vector<fc::ecc::private_key> recv_keys;
    auto keychain =  tmp_profile->get_keychain();

    std::vector<identity>   idents = tmp_profile->identities();
    for( auto itr = idents.begin(); itr != idents.end(); ++itr )
    {
       recv_keys.push_back( keychain.get_identity_key( itr->bit_id ) );
    }
    my->_bitchat_client->set_receive_keys( recv_keys );

    return my->_profile = tmp_profile;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }


  profile_ptr                 application::create_profile( const profile_config& cfg, const std::string& password )
  { try {
    fc::create_directories( my->_profile_dir  );

    // note: stored in temp incase create throws.
    auto tmp_profile = std::make_shared<profile>();

    tmp_profile->create( my->_profile_dir / "default", cfg, password );
    tmp_profile->open( my->_profile_dir / "default", password );

    return my->_profile = tmp_profile;

  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

                              
  fc::optional<bitname::name_record>   application::lookup_name( const std::string& name )
  { try {
    FC_ASSERT( my->_config );
    return my->_bitname_client->lookup_name( name );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("name",name) ) }

  fc::optional<bitname::name_record>   application::reverse_name_lookup( const fc::ecc::public_key& key )
  { try {
    FC_ASSERT( my->_config );
    return my->_bitname_client->reverse_name_lookup( key );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("key",key) ) }

  void                        application::mine_name( const std::string& name, const fc::ecc::public_key& key, float effort )
  {
     FC_ASSERT( my->_config );
     my->_bitname_client->mine_name( name, key );
  }

  void  application::send_contact_request( const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  {
     FC_ASSERT( my->_config );
  }

  void  application::send_email( const bitchat::private_email_message& email, 
                                 const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  { try {
     FC_ASSERT( my->_config );

     bitchat::decrypted_message msg( email );
     msg.sign(from);
     my->_bitchat_client->send_message( msg, to, 0/* chan 0 */ );

  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

  void  application::send_text_message( const bitchat::private_text_message& txtmsg, 
                                        const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  { try {
     FC_ASSERT( my->_config );

     bitchat::decrypted_message msg( txtmsg );
     msg.sign(from);
     my->_bitchat_client->send_message( msg, to, 0/* chan 0 */ );

  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

  void  application::add_node( const fc::ip::endpoint& remote_ep )
  { try {
     FC_ASSERT( my->_config );
     my->_server->connect_to(remote_ep);
  } FC_RETHROW_EXCEPTIONS( warn, "", ("endpoint",remote_ep) ) }


  void application::quit()
  { try {
       my.reset();
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }
 
  application_ptr application::instance()
  {
      static application_ptr app = std::make_shared<application>();
      return app;
  }
} // namespace bts
