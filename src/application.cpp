#include <bts/application.hpp>

namespace bts {

  namespace detail
  {
    class application_impl
    {
       public:
          application_impl()
          :_delegate(nullptr){}

          application_delegate* _delegate;
          application_config    _config;
          profile_ptr           _profile;
    };
  }

  application::application()
  :my( new detail::application_impl() )
  {
  }

  application::~application(){}

  void application::configure( const application_config& cfg )
  {
     my->_config = cfg;
  }

  application_config application::get_configuration()const
  {
     return my->_config;
  }

  void      application::set_application_delegate( application_delegate* del )
  {
     my->_delegate = del;
  }

  profile_ptr                 application::get_profile()
  {
    return my->_profile;
  }

  profile_ptr                 application::load_profile( const std::string& password )
  {
    return my->_profile;
  }

  profile_ptr                 application::create_profile( const profile_config& cfg, const std::string& password )
  {
    return my->_profile;
  }
                              
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
