#include <bts/bitchat/bitchat_client.hpp>
#include <bts/bitchat/bitchat_channel.hpp>
#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>
#include <unordered_map>
#include <map>

namespace bts { namespace bitchat {
   using network::channel_id;
   namespace detail 
   { 
     class client_impl : public bts::bitchat::channel_delegate
     {
       public:
         bts::peer::peer_channel_ptr peers;
         bitchat_delegate*           del;
   
         std::map<std::string,contact>                      contacts;
         std::map<std::string,identity>                     idents;
         std::unordered_map<std::string,std::string>        key_to_contact;  
         std::unordered_map<std::string,std::string>        key_to_idents;  

         std::unordered_map<uint16_t, bitchat::channel_ptr> channels;
         
         virtual void handle_message( const encrypted_message& pm, const network::channel_id& c )
         {
              decrypted_message dm;
              // check for messages to me
              for( auto itr = idents.begin(); itr != idents.end(); ++itr )
              {
                 if( pm.decrypt( itr->second.key, dm ) )
                 {
                    // TODO: pass contact to handle data msg?
                    handle_decrypted_message( dm, c, itr->second );
                    return;
                 }
                 if( pm.decrypt( itr->second.broadcast, dm ) )
                 {
                    handle_self_broadcast( dm, itr->second );
                    return;
                 }
              }

              // check for messages from others
              for( auto itr = contacts.begin(); itr != contacts.end(); ++itr )
              {
                 if( itr->second.recv_broadcast )
                 {
                   if( pm.decrypt( *itr->second.recv_broadcast, dm ) )
                   {
                      handle_decrypted_broadcast( dm, itr->second );
                      return;
                   }
                 }
              
              }
              ilog( "unable to decrypt message ${m}", ("m", pm.id()) );
         }

         void handle_self_broadcast( const decrypted_message& dm, const identity& to )
         {
            wlog( "TODO: implement handle_self_broadcast ${to}", ("to",to) );
         }

         void handle_decrypted_message( const decrypted_message& m, 
                                        const channel_id& c, const identity& to )
         {
            switch( (private_message_type)m.msg_type )
            {
               case text_msg:
                   handle_text_message( m.as<private_text_message>(),
                                        m.from_key, to );
                   break;
               case contact_request_msg:
                   break;
               case contact_auth_msg:
                   break;
               case status_msg:
                   break;
               default:
                 wlog( "unknown private message type ${t}", ("t", m.msg_type)("message",m));
            }
         }

         void handle_decrypted_broadcast( const decrypted_message& m, const contact& from )
         {
             if( !m.from_key || m.from_key->serialize() != from.key.serialize() )
             {
                wlog( "Someone other than ${contact} is broadcasting on ${contact}'s address",
                      ("contact", from.label )("msg", m) );
                return; // ignore it!
             }
             
             switch( (private_message_type)m.msg_type )
             {
                case text_msg:
                {
                    auto tm = m.as<private_text_message>();
                    del->received_broadcast( tm.msg, from );
                    return;
                }
                case contact_request_msg:
                    wlog( "contact request should not be sent as a broadcast" );
                    break;
                case contact_auth_msg:
                    wlog( "contact auth should not be sent as a broadcast" );
                    break;
                case status_msg:
                {
     //               auto st  = m.as<private_status_message>();
      //              handle_status_message( st, from );
                    return;
                }
                default:
                  wlog( "unknown private message type ${t}", ("t", m.msg_type)("message",m));
             }
             
         }


         void handle_status_message( const private_status_message& pm, const contact& c )
         {
            switch( (account_status)pm.status )
            {
                case active:
                  del->contact_signon( c );
                  return;
                case away:
                  del->contact_away( c, pm.status_message );
                  return;
                case idle:
                  del->contact_idle( c );
                  return;
                case signoff:
                  del->contact_signoff( c );
                  return;
                case unknown:
                   wlog( "unknown contct state.." );
                   return;
                default:
                   wlog( "unhandled contct state.." );
            }
         }

         void handle_text_message( const private_text_message& m, 
                                   const fc::optional<fc::ecc::public_key>& from,
                                   const identity& to )
         {
             if( from )
             {
                address from_adr = address(*from);
                std::string from_str(from_adr);
                
                auto itr = key_to_contact.find( from_str );
                if( itr == key_to_contact.end() )
                {
                    contacts[from_str] = contact( from_str, *from );
                    key_to_contact[from_str] = from_str;
                }
                ilog( "from: ${from}", ("from",from_str) ); 
                del->received_message( m.msg, to, contacts[itr->second]);
            }
            else
            {
                del->received_anonymous_message( m.msg, to );
            }
         }


         void subscribe_to_channel( const channel_id& c )
         {
             FC_ASSERT( c.proto == network::chat_proto );
             auto itr = channels.find( c.chan );
             if( itr == channels.end() )
             {
                channels[c.chan] = std::make_shared<bitchat::channel>( peers, c, this );
             }
         }
     }; // class client_impl

   } // namespace detail 
   
   client::client( const bts::peer::peer_channel_ptr& p, bitchat_delegate* d )
   :my( new detail::client_impl() )
   {
       assert( d != nullptr );
       my->peers = p;
       my->del   = d;

       // By default subscribe to channel 0 where everyone is subscribed.
       my->subscribe_to_channel( channel_id( network::chat_proto, 0 ) );
   }
   
   client::~client()
   {
      ilog( "" );
   }



   void        client::add_identity( const identity& id )
   {
       ilog( "${id}", ("id",id) );
       my->idents[id.label] = id;
       my->key_to_idents[address(id.key.get_public_key())] = id.label;
   }

   identity    client::get_identity( const std::string& label )
   {
       auto itr = my->idents.find(label);
       if( itr == my->idents.end() )
       {
         FC_THROW_EXCEPTION( key_not_found_exception, 
                    "Unable to find identity with label ${label}", ("label", label) );
       }
       return itr->second;
   }

   void        client::add_contact( const contact& c )
   {
       ilog( "${c}", ("c",c) );
       my->contacts[c.label] = c;
       my->key_to_contact[address(c.key)] = c.label;
   }

   contact     client::get_contact( const std::string& label )
   {
       auto itr = my->contacts.find(label);
       if( itr == my->contacts.end() )
       {
         FC_THROW_EXCEPTION( key_not_found_exception, 
                    "Unable to find contact with label ${label}", ("label", label) );
       }
       return itr->second;
   }

   void client::send_message( const std::string& msg, 
                      const contact& to, 
                      const identity& from )
   {
      try {

       decrypted_message dm = decrypted_message( private_text_message( msg ) );
       dm.sign( from.key );
       encrypted_message em = dm.encrypt( to.key );
       em.timestamp = fc::time_point::now();

       
       FC_ASSERT( to.send_channels.size() != 0 );
       auto itr = my->channels.find( to.send_channels.front() );

       if( itr == my->channels.end() )
       {
          FC_THROW_EXCEPTION( exception, "unable to find channel ${c}", ("c",to.send_channels.front()) );
       }

       // TODO: calculate proof of work and perform the proof of work..

       itr->second->broadcast( std::move(em) );

      } FC_RETHROW_EXCEPTIONS( warn, "unable to send message '${msg}' to ${contact}", 
                               ("msg",msg)
                               ("contact",to.label)
                               ("to",to)
                               ("from",from) );

   }

   void client::request_contact( const identity& id, const std::string& msg )
   {
   }

   void client::broadcast_away( const identity& id, const std::string& msg )
   {
   }

   void client::broadcast_signoff( const identity& id )
   {
   }

   void client::broadcast_signon( const identity& id )
   {
   }

   std::string to_address( const fc::ecc::public_key& e )
   {
      auto dat = e.serialize();
      return fc::to_base58( dat.data, sizeof(dat) );
   }

   fc::ecc::public_key from_address( const std::string& s )
   {
      auto dat = fc::from_base58( s );
      fc::ecc::public_key_data d;
      FC_ASSERT( dat.size() == sizeof(fc::ecc::public_key_data) );
      memcpy( d.data, dat.data(), dat.size() );
      return fc::ecc::public_key(d);
   }

   address::address( const fc::ecc::public_key& k )
   {
      key   = k.serialize();
      check = fc::city_hash64( key.data, sizeof(key) );
   }
   address::address( const std::string& a )
   {
      auto dat = fc::from_base58(a);
      fc::datastream<const char*> ds(dat.data(),dat.size());
      fc::raw::unpack( ds, *this );
      uint32_t c = fc::city_hash64( key.data, sizeof(key) );
      if( c != check )
      {
         FC_THROW_EXCEPTION( exception, "invalid bitchat address checksum", ("address",*this) );
      }
   }

   address::operator std::string()const
   {
      auto vec = fc::raw::pack(*this);
      return fc::to_base58( vec.data(), vec.size() );
   }

   address::operator fc::ecc::public_key()const
   {
      uint32_t c = fc::city_hash64( key.data, sizeof(key) );
      if( c != check )
      {
         FC_THROW_EXCEPTION( exception, "invalid bitchat address checksum", ("address",*this) );
      }
      return fc::ecc::public_key(key);
   }

} }
