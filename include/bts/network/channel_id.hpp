#pragma once
#include <fc/io/varint.hpp>
#include <string.h>

namespace bts { namespace network {

  enum channel_proto 
  {
     null_proto = 0, ///< used for null / padding  
     peer_proto = 1, ///< used for discovery / configuration
     chat_proto = 2, ///< used for chat messages
     name_proto = 3, ///< used for bitname messages
     mail_proto = 4  ///< used for email messages
  };

  struct channel_id
  {
     channel_id( channel_proto p = null_proto, uint16_t c = 0 )
     :proto(p),chan(c)
     {
     }

     explicit channel_id( uint32_t u )
     {
        chan  = u >> 8;
        proto = (channel_proto)(u & 0x000000ff);
     }
     /** 
      *   @brief defines the protocol being being used on a
      *          channel.
      */
     channel_proto proto;
     /**
      *    @brief identifies the channel number
      */
     uint16_t chan;

     friend bool operator == ( const channel_id& a, const channel_id& b )
     {
      return a.id() == b.id();
     }
     friend bool operator != ( const channel_id& a, const channel_id& b )
     {
      return a.id() != b.id();
     }
     friend bool operator < ( const channel_id& a, const channel_id& b )
     {
      return a.id() < b.id();
     }

     uint32_t id()const
     {
        return (uint32_t(chan) << 8) | proto;
     }
  };
}}  // namespace bts::network
namespace fc { namespace raw {
    template<typename Stream>
    inline void pack( Stream& s, const bts::network::channel_proto& tp )
    {
       uint8_t p = (uint8_t)tp;
       s.write( (const char*)&p, sizeof(p) );
    }

    template<typename Stream>
    inline void unpack( Stream& s, bts::network::channel_proto& tp )
    {
       uint8_t p;
       s.read( (char*)&p, sizeof(p) );
       tp = bts::network::channel_proto(p);
    }
}}


#include <fc/reflect/reflect.hpp>
FC_REFLECT_ENUM( bts::network::channel_proto, 
    (null_proto)
    (peer_proto)
    (chat_proto)
    (name_proto)
    (mail_proto)
)

FC_REFLECT( bts::network::channel_id, 
    (proto)
    (chan) 
    )



