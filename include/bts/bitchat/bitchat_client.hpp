#pragma once
#include <bts/peer/peer_channel.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/time.hpp>
#include <memory>

namespace bts { namespace bitchat 
{
    namespace detail { class client_impl; }

    class client_delegate
    {
       public:
         virtual void bitchat_message_received( const decrypted_message& m ){}          
    };

    /**
     *  Provides checked conversion of public key to / from bitchat address
     *  TODO: move this someplace else, this type of address representation
     *  is only useful for user-facing interfaces, under the hood we just
     *  deal with public keys.
    struct address
    {
       address( const fc::ecc::public_key& k );
       address( const std::string& a );

       operator std::string()const;
       operator fc::ecc::public_key()const;

       uint32_t                 check;
       fc::ecc::public_key_data key;
    };
     */


    /**
     *  Provides a simple chat client that hides both the
     *  sender and receiver of the message.  
     *
     */
    class client
    {
        public:
          client( const bts::peer::peer_channel_ptr& s, client_delegate* d );
          ~client();

          void configure( const fc::path& dir );

          /** sets the private keys that are used to receive incoming messages*/
          void set_receive_keys( const std::vector<fc::ecc::private_key>& recv_keys );

          void join_channel( uint16_t chan_num );
          void send_message( const decrypted_message& m, const fc::ecc::public_key& to, uint16_t chan = 0);

        private:
          std::unique_ptr<detail::client_impl> my;
    };
    
    typedef std::shared_ptr<client> client_ptr;
} } // bts::bitchat


