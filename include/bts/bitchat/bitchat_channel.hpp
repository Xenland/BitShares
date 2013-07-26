#pragma once
#include <bts/peer/peer_channel.hpp>

namespace bts { namespace bitchat {

   namespace detail { class channel_impl; }

   class encrypted_message;
   
   class channel_delegate
   {
     public:
       virtual ~channel_delegate(){}

       virtual void handle_message( const encrypted_message& pm, const network::channel_id& c ) = 0;
   
   };
   
   /**
    *  @brief Implements the message broadcast/routing for a single channel.
    *
    *  This class is agnostic to anything but message routing.  It does not
    *  handle private messages nor attempt to decrypt anything, instead it
    *  manages the bandwidth and passes all messages it receives through
    *  to the delegate.
    */
   class channel 
   {
     public:
       channel( const bts::peer::peer_channel_ptr& peers, const network::channel_id& c, channel_delegate* d );
       ~channel();
  
       network::channel_id get_id()const;

       void broadcast( encrypted_message&& m );

     private:
       std::shared_ptr<detail::channel_impl> my;
   };

   typedef std::shared_ptr<channel> channel_ptr;

}  } // bts::bitchat
