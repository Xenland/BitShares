#include <bts/peer/peer_messages.hpp>

namespace bts { namespace peer {
  
const message_code config_msg::type;
const message_code known_hosts_msg::type;
const message_code subscribe_msg::type;
const message_code unsubscribe_msg::type;
const message_code get_subscribed_msg::type;
const message_code error_report_msg::type;
const message_code get_known_hosts_msg::type;

} } // bts::peer
