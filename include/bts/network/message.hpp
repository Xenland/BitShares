#pragma once
#include <bts/network/channel_id.hpp>
#include <fc/time.hpp>
#include <fc/array.hpp>
#include <fc/io/varint.hpp>
#include <fc/network/ip.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace network {

  /**
   *  Defines an 8 byte header that is always present because the minimum encrypted packet
   *  size is 8 bytes (blowfish).  The maximum message size is 16 MB.  The channel,
   *  and message type is also included because almost every channel will have a message type
   *  field and we might as well include it in the 8 byte header to save space.
   */
  struct message_header
  {
     channel_id channel()const { return channel_id( (channel_proto)proto, chan_num ); }

     uint32_t  size:24;   // number of bytes in message, max 16 MB per message
     uint8_t   proto;     // protocol of the channel
     uint16_t  chan_num;  // channel identifier, 60K channels per protocol max
     uint16_t  msg_type;  // every channel gets a 16 bit message type specifier

  };
  static_assert( sizeof(message_header) == sizeof(uint64_t), "message header fields should be tightly packed" );


  /**
   *  Abstracts the process of packing/unpacking a message for a 
   *  particular channel.
   */
  struct message : public message_header
  {
     std::vector<char> data;

     message(){}

     message( message&& m )
     :message_header(m),data( std::move(m.data) ){}

     message( const message& m )
     :message_header(m),data( m.data ){}

     /**
      *  Assumes that T::type specifies the message type
      */
     template<typename T>
     message( const T& m, const channel_id cid = channel_id() ) 
     {
        proto    = cid.proto;
        chan_num = cid.chan;
        msg_type = T::type;

        fc::datastream<size_t> ps;
        fc::raw::pack( ps, m );
        data.resize( 8*((ps.tellp() + 7)/8) ); // round up to nearest 8 byte bounds.

        // TODO: is it safe for the trailing garbage bytes to be uninitialized?
        if( data.size() )
        {
           fc::datastream<char*> ds( data.data(), data.size() );
           fc::raw::pack( ds, m );
        }
        size = data.size();
     }
    
     /**
      *  Automatically checks the type and deserializes T in the
      *  opposite process from the constructor.
      */
     template<typename T>
     T as()const 
     {
       try {
        FC_ASSERT( msg_type == T::type );
        T tmp;
        if( data.size() )
        {
           fc::datastream<const char*> ds( data.data(), data.size() );
           fc::raw::unpack( ds, tmp );
        }
        else
        {
           // just to make sure that tmp shouldn't have any data
           fc::datastream<const char*> ds( nullptr, 0 );
           fc::raw::unpack( ds, tmp );
        }
        return tmp;
       } FC_RETHROW_EXCEPTIONS( warn, 
            "error unpacking network message as a '${type}'  ${x} != ${msg_type}", 
            ("type", fc::get_typename<T>::name() )
            ("x", T::type)
            ("msg_type", msg_type)
            );
     }
  };

} } // bts::network

namespace fc 
{ 
   namespace raw {

      /**
       *  @read write in blocks of at least 8 bytes for encryption purposes
       */
      template<typename Stream>
      inline void pack( Stream& s, const bts::network::message& m )
      {
         FC_ASSERT( m.size % 8 == 0 );
         s.write( (char*)&m, sizeof( bts::network::message_header ) );
         if( m.size )
         {
            s.write( m.data.data(), m.data.size() );
         }
      }
  
      /**
       *  @read read in blocks of at least 8 bytes for decryption purposes
       */
      template<typename Stream>
      inline void unpack( Stream& s, bts::network::message& m )
      {
         s.read( (char*)&m, sizeof( bts::network::message_header ) );
         FC_ASSERT( m.size % 8 == 0 );
         if( m.size > 0 )
         {
             m.data.resize( m.size ); // note: size is 24 bit, max 16 MB thus safe to use without check
             s.read( m.data.data(), m.data.size() );
         }
      }
  
   } // namespace raw
} // namespace fc


