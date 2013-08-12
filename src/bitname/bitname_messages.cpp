#include <bts/bitname/bitname_messages.hpp>

namespace bts { namespace bitname {

const message_type name_inv_message::type      = name_inv_msg;
const message_type block_inv_message::type     = block_inv_msg;
const message_type get_name_inv_message::type  = get_name_inv_msg;
const message_type get_block_inv_message::type = get_block_inv_msg;
const message_type get_headers_message::type   = get_headers_msg;
const message_type get_block_message::type     = get_block_msg;
const message_type get_name_message::type      = get_name_msg;
const message_type name_message::type          = name_msg;
const message_type block_message::type         = block_msg;
const message_type headers_message::type       = headers_msg;

} } // bts::bitname
