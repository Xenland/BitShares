#pragma once
#include <bts/network/channel_id.hpp>
#include <bts/peer/peer_host.hpp>
#include <fc/time.hpp>
#include <fc/network/ip.hpp>
#include <unordered_set>

namespace bts { namespace peer {
  /// these belong as part of the peer proto channel, not part of
  ///  the mssage class.
  enum message_code
  {
     generic         = 0,
     config          = 1,
     known_hosts     = 2,
     error_report    = 3,
     subscribe       = 4,
     unsubscribe     = 5,
     get_known_hosts = 6,
     get_subscribed  = 7
  };

  struct config_msg
  {
      static const message_code type = message_code::config;
      /** 
       *  A list of features supported by this client.
       */
      std::unordered_set<std::string> supported_features;
      fc::ip::endpoint                public_contact;
  };


  struct known_hosts_msg
  {
     static const message_code type = message_code::known_hosts;
      known_hosts_msg(){}
      known_hosts_msg( std::vector<host> h )
      :hosts( std::move(h) ){}
      std::vector<host> hosts;
  };

  struct subscribe_msg
  {
     static const message_code type = message_code::subscribe;
     subscribe_msg(){}
     subscribe_msg( std::vector<network::channel_id> chans )
     :channels( std::move(chans) ){}

     std::vector<network::channel_id> channels;
  };

  struct unsubscribe_msg
  {
     static const message_code type = message_code::unsubscribe;
     std::vector<network::channel_id> channels;
  };

  struct get_subscribed_msg 
  {
     static const message_code type = message_code::get_subscribed;
  };

  struct get_known_hosts_msg
  {
      static const message_code type = message_code::get_known_hosts;
      get_known_hosts_msg(){}
  };

  struct error_report_msg
  {
     static const message_code type = message_code::error_report;
     uint32_t     code;
     std::string  message;
  };

} } // bts::peer

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::peer::config_msg,       
    (supported_features)
    (public_contact)
    )

FC_REFLECT_ENUM( bts::peer::message_code,
  (generic)
  (config)
  (known_hosts)
  (error_report)
  (subscribe)
  (unsubscribe)
  (get_known_hosts)
  (get_subscribed)
  )
FC_REFLECT( bts::peer::known_hosts_msg,  (hosts) )
FC_REFLECT( bts::peer::error_report_msg, (code)(message) )
FC_REFLECT( bts::peer::subscribe_msg,    (channels ) )
FC_REFLECT( bts::peer::unsubscribe_msg,  (channels ) )
FC_REFLECT( bts::peer::get_known_hosts_msg, BOOST_PP_SEQ_NIL ); 

