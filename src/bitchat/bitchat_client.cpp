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
         client_delegate*           del;

         std::vector<fc::ecc::private_key>                  _recv_keys;
         std::unordered_map<uint16_t, bitchat::channel_ptr> _channels;
         fc::path                                           _data_dir;
         
         virtual void handle_message( const encrypted_message& cipher_message, const network::channel_id& c )
         {
              decrypted_message clear_message;
              for( auto itr = _recv_keys.begin(); itr != _recv_keys.end(); ++itr )
              {
                   if( cipher_message.decrypt( *itr, clear_message ) )
                   {
                      if( del )
                      {
                         del->bitchat_message_received( clear_message );
                         return;
                      }
                   }
              }
              ilog( "unable to decrypt message ${m}", ("m", cipher_message.id()) );
         }


         void subscribe_to_channel( const channel_id& c )
         {
             FC_ASSERT( c.proto == network::chat_proto );
             auto itr = _channels.find( c.chan );
             if( itr == _channels.end() )
             {
                _channels[c.chan] = std::make_shared<bitchat::channel>( peers, c, this );
                _channels[c.chan]->configure( channel_config( _data_dir / ("channel"+ fc::to_string(uint64_t(c.chan)) ) ) );
             }
         }
     }; // class client_impl

   } // namespace detail 
   
   client::client( const bts::peer::peer_channel_ptr& peers, client_delegate* chat_delegate )
   :my( new detail::client_impl() )
   {
       assert( chat_delegate != nullptr );
       my->peers = peers;
       my->del   = chat_delegate;

       // By default subscribe to channel 0 where everyone is subscribed.
       my->subscribe_to_channel( channel_id( network::chat_proto, 0 ) );
   }
   
   client::~client()
   {
      ilog( "" );
   }
   void client::join_channel( uint16_t chan_num )
   {
       my->subscribe_to_channel( channel_id( network::chat_proto, chan_num ) );
   }

   void client::send_message( const decrypted_message& m, const fc::ecc::public_key& to, uint16_t chan )
   { try {
        auto cipher_message = m.encrypt( to );
        cipher_message.timestamp = fc::time_point::now();

        auto itr = my->_channels.find( chan );
        if( itr == my->_channels.end() )
        {
          FC_THROW_EXCEPTION( exception, "unable to find bitchat channel ${c}", ("c",chan)("message",m) );
        }
        itr->second->broadcast( std::move(cipher_message) );
   } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",m) ) }

   void client::set_receive_keys( const std::vector<fc::ecc::private_key>& recv_keys )
   {
     my->_recv_keys = recv_keys;
   }

   void client::configure( const fc::path& dir )
   { try {
      my->_data_dir = dir;
      fc::create_directories(dir);
      for( auto itr = my->_channels.begin(); itr != my->_channels.end(); ++itr )
      {
         itr->second->configure( channel_config( dir / ("channel"+ fc::to_string( uint64_t(itr->first) ) ) ) );
      }
   } FC_RETHROW_EXCEPTIONS( warn, "", ("dir",dir) ) }

} }
