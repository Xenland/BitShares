#pragma once
#include <bts/bitchat/bitchat_private_message.hpp>

namespace bts { namespace bitchat {

  // other protocol messages... not encapsulated by data_message
  struct inv_message 
  {
      static const message_type type = message_type::inv_msg;
      std::vector<mini_pow>  items;
  };

  struct get_priv_message
  {
      static const message_type type = message_type::get_priv_msg;
      get_priv_message(){}
      get_priv_message( const mini_pow& p )
      {
        items.push_back(p);
      }
      std::vector<mini_pow>  items;
  };

  /**
   *  Used to request all inventory after the given time.
   */
  struct get_inv_message
  {
      static const message_type type = message_type::get_inv_msg;
      fc::time_point_sec after;
  };

} } // bts::bitchat

FC_REFLECT( bts::bitchat::inv_message,      (items) )
FC_REFLECT( bts::bitchat::get_priv_message, (items) )
FC_REFLECT( bts::bitchat::get_inv_message,  (after) )
