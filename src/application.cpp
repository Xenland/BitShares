#include <bts/application.hpp>
#include <bts/bitname/bitname_client.hpp>
#include <fc/reflect/variant.hpp>

#include <fc/log/logger.hpp>

namespace bts {

  namespace detail
  {
    class application_impl : public bts::bitname::client_delegate
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




          void bitname_block_added( const bts::bitname::name_block& h )
          {

          }
         
          void bitname_header_pending( const bts::bitname::name_header& h )
          {
              ilog( "${h}", ("h",h) );
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

    return my->_profile = tmp_profile;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }


  profile_ptr                 application::create_profile( const profile_config& cfg, const std::string& password )
  { try {
    fc::create_directories( my->_profile_dir );

    // note: stored in temp incase create throws.
    auto tmp_profile = std::make_shared<profile>();

    tmp_profile->create( my->_profile_dir / "default", cfg, password );

    return my->_profile = tmp_profile;

  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

                              
  fc::optional<bitname::name_record>   application::lookup_name( const std::string& name )
  {
    fc::optional<bitname::name_record> rec;

    return rec;
  }
  fc::optional<bitname::name_record>   application::reverse_name_lookup( const fc::ecc::public_key& key )
  {
    fc::optional<bitname::name_record> rec;

    return rec;
  }
  void                        application::mine_name( const std::string& name, const fc::ecc::public_key& key, float effort )
  {
  }

  void  application::send_contact_request( const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  {
  }

  void  application::send_email( const bitchat::private_email_message& email, const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  {
  }

  void  application::send_text_message( const bitchat::private_text_message& txtmsg, const fc::ecc::public_key& to, const fc::ecc::private_key& from )
  {
  }

} // namespace bts
