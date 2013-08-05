#pragma once
#include <bts/peer/peer_channel.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/time.hpp>
#include <memory>

namespace bts { namespace bitchat 
{
    namespace detail { class client_impl; }

    struct contact
    {
        contact( std::string l, fc::ecc::public_key k )
        :label( std::move(l) ), key( std::move(k) ){}
        contact(){}

        std::string                        label;
        fc::ecc::public_key                key;
        ///< contact broadcasts here,  I should listen
        fc::optional<fc::ecc::private_key> recv_broadcast;
        std::vector<uint16_t>              send_channels;
    };

    struct contact_status
    {
        contact               ident;
        std::string           away_message;
        bool                  online;
    };

    struct identity 
    {
        std::string            label;
        fc::ecc::private_key   key;
        fc::ecc::private_key   broadcast; 
        std::vector<uint16_t>  recv_channels;
    };

    class bitchat_delegate
    {
       public:
         virtual ~bitchat_delegate(){}
         virtual void contact_signon( const contact& id ){};
         virtual void contact_signoff( const contact& id ){};
         virtual void contact_idle( const contact& id ){};
         virtual void contact_away( const contact& id, 
                                   const std::string& msg ){};

         virtual void contact_request( const contact& id, 
                                      const std::string& msg ){};

         virtual void received_message( const std::string& msg, 
                                        const identity& to,
                                        const contact& from ){};
         virtual void received_anonymous_message( const std::string& msg, 
                                                  const identity& to){};

         virtual void received_broadcast( const std::string& msg, 
                                          const contact& from ){};
        
         /** perhaps we should warn the user that someone else is using
          *  their keys to communicate... 
          */
         virtual void received_from_self( const std::string& msg,
                                           const identity& to ){};
    };

    std::string         to_address( const fc::ecc::public_key& e );
    fc::ecc::public_key from_address( const std::string& s );


    /**
     *  Provides checked conversion of public key to / from bitchat address
     */
    struct address
    {
       address( const fc::ecc::public_key& k );
       address( const std::string& a );

       operator std::string()const;
       operator fc::ecc::public_key()const;

       uint32_t                 check;
       fc::ecc::public_key_data key;
    };


    /**
     *  Provides a simple chat client that hides both the
     *  sender and receiver of the message.  
     *
     */
    class client
    {
        public:
          struct config
          {
             std::vector<identity> idents; // my ids
             std::vector<contact>  contacts; ///< information about my contacts
          };

          client( const bts::peer::peer_channel_ptr& s, bitchat_delegate* d );
          ~client();

          void               add_identity( const identity& id );
          identity           get_identity( const std::string& label );

          void               add_contact( const contact& c );
          contact            get_contact( const std::string& label );

          void send_message( const std::string& msg, 
                             const contact& to, 
                             const identity& from );

          void request_contact( const identity& id, const std::string& msg );

          void broadcast_away( const identity& id, const std::string& msg );
          void broadcast_signoff( const identity& id );
          void broadcast_signon( const identity& id );

        private:
          std::unique_ptr<detail::client_impl> my;
    };
    
    typedef std::shared_ptr<client> client_ptr;
} } // bts::bitchat

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::bitchat::client::config,  (idents)(contacts) )
FC_REFLECT( bts::bitchat::identity, (label)(key)(broadcast)(recv_channels) )
FC_REFLECT( bts::bitchat::contact,  (label)(key)(recv_broadcast)(send_channels) )
FC_REFLECT( bts::bitchat::address,  (key)(check) )


