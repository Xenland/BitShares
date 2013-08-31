#pragma once
#include <bts/network/channel_id.hpp>
#include <fc/io/raw.hpp>
#include <fc/thread/future.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/time.hpp>
#include <fc/optional.hpp>
#include <fc/io/enum_type.hpp>

namespace bts { namespace bitchat {
    using network::channel_id;

    enum message_type
    {
       inv_msg        = 1, ///< publishes known inventory
       get_inv_msg    = 2, ///< requests inventory
       get_priv_msg   = 3, ///< sent to request an private message in the inventory
       encrypted_msg  = 4  ///< a message encrypted to unknown receip (sent in reponse to get_priv_msg)
    };

    enum compression_type
    {
       no_compression   = 0,
       smaz_compression = 1,
       lzma_compression = 2
    };

    enum encryption_type
    {
       no_encryption       = 0,
       blowfish_encryption = 1,
       twofish_encryption  = 2,
       aes_encryption      = 3
    };

    struct decrypted_message;

    /**
     *  An encrypted message is encoded with a particular public key destination in
     *  mind.  Each message establishes a new ECDH key pair and one-time shared secret 
     *  to establish the blowfish decryption key.
     *
     *  TODO: add move semantics to encrypted_message
     */
    struct encrypted_message 
    {
        static const message_type type = encrypted_msg;
        encrypted_message();

        uint16_t                                      nonce; ///< increment timestamp after 63K tests
        fc::time_point_sec                            timestamp;
        fc::ecc::public_key                           dh_key;
        fc::uint160_t                                 check;
        std::vector<char>                             data;

        fc::uint128        id()const;

        /**
         *  This method will increment the nonce or timestamp until bts::difficulty(id()) > tar_per_kb*(data.size()/1024).
         *
         *  @return a future object that can be used to cancel the proof of work, result true if target found.
         */
        fc::future<bool>   do_proof_work( uint64_t tar_per_kb );
        bool               decrypt( const fc::ecc::private_key& with, decrypted_message& m )const;
    };


    /** content of private_message data */
    enum private_message_type
    {
       unknown_msg          = 0,
       text_msg             = 1,
       email_msg            = 2,
       contact_request_msg  = 3,
       contact_auth_msg     = 4,
       status_msg           = 5
    };

    /**
     *  A decrypted message has a type, payload, timestamp and signature that allow us to
     *  derive the public key of the signer.  It is designed to be easily populated with
     *  many different types of messages that get serialized/deserialized into the data field.
     *
     *  TODO: add move semantics to decrypted_message
     */
    struct decrypted_message
    {
        template<typename T>
        decrypted_message( const T& msg )
        {
           msg_type = T::type;
           data = fc::raw::pack( msg );
        }

        template<typename T>
        T as()const
        {
           if( msg_type != T::type )
           {
              FC_THROW_EXCEPTION( bad_cast_exception, "Unable to cast ${msg_type} to ${type}",
                                                      ("msg_type",msg_type)("type",T::type) );
           }
           return fc::raw::unpack<T>( data );
        }

        decrypted_message();
        encrypted_message                    encrypt( const fc::ecc::public_key& to )const;
        decrypted_message&                   sign( const fc::ecc::private_key& from );
        fc::sha256                           digest()const;

        /** type of the decrypted, uncompressed message */
        fc::enum_type<fc::unsigned_int,private_message_type>  msg_type;
        /** any compression applied to data before encrypting it */
        fc::enum_type<fc::unsigned_int,compression_type>      compression_format;
        /** any additional encryption applied to data */
        fc::enum_type<fc::unsigned_int,encryption_type>       encryption_method;
        std::vector<char>                                     data;
                                                         
        fc::time_point_sec                                    sig_time; 
        fc::optional<fc::ecc::compact_signature>              from_sig;
                                                               
        fc::optional<fc::ecc::public_key>                     from_key;
        fc::optional<fc::ecc::private_key>                    decrypt_key;
    };

    struct private_text_message 
    {
       static const private_message_type  type;

       private_text_message( std::string m = std::string())
       :msg( std::move(m) ){}
      
       std::string      msg;
    };

    struct private_contact_request_message 
    {
       static const private_message_type  type;

       std::string from_name;
       std::string greeting_message;///< message introducing name/key
       channel_id  from_channel;    ///< channel where from_name can be contacted
    };

    struct attachment
    {
      std::string       filename;
      std::vector<char> body;
    };

    struct private_email_message
    {
       static const private_message_type  type;
       std::string                        subject;
       std::string                        body;
       std::vector<attachment>            attachments;
    };


    struct private_contact_auth_message 
    {
       static const private_message_type  type;

       std::string                        auth_text;         ///< "sorry, ok, ..."
       fc::uint128                        min_work;          ///< how much work is required to contact this individual
       fc::time_point_sec                 expires;           ///< specifies when the channel list and broadcast key will expire.
       std::vector<network::channel_id>   listen_channels;   ///< channel where from_name can be contacted
       fc::optional<fc::ecc::private_key> broadcast_key;     ///< key used by this contact for broadcasting updates, updates
                                                             ///  are only valid if signed with the actual public key, anyone
                                                             ///  else 'publishing' with this broadcast key should be ignored.
    };


    enum account_status 
    {  
      unknown = 0, 
      active  = 1, 
      away    = 2,
      idle    = 3, 
      signoff = 4
    };

    /**
     *  Used to broadcast to those who you have shared your broadcast_key with that you
     *  are online, away, etc.
     */
    struct private_status_message 
    {
       static const private_message_type  type;

       private_status_message( account_status s = unknown, std::string m = std::string() )
       :status( s ), status_message( std::move(m) ){}
       fc::enum_type<fc::unsigned_int,account_status>  status;
       std::string     status_message;
    };

} }  // namespace btc

#include <fc/reflect/reflect.hpp>
FC_REFLECT_ENUM( bts::bitchat::account_status, (unknown)(active)(away)(idle) )
FC_REFLECT_ENUM( bts::bitchat::message_type, (inv_msg)(get_inv_msg)(get_priv_msg)(encrypted_msg) )
FC_REFLECT_ENUM( bts::bitchat::private_message_type, (unknown_msg)(text_msg)(email_msg)(contact_request_msg)(contact_auth_msg)(status_msg) )
FC_REFLECT_ENUM( bts::bitchat::compression_type, (no_compression)(smaz_compression)(lzma_compression) )
FC_REFLECT_ENUM( bts::bitchat::encryption_type, (no_encryption)(blowfish_encryption)(twofish_encryption)(aes_encryption) )
FC_REFLECT( bts::bitchat::attachment, (filename)(body) )
FC_REFLECT( bts::bitchat::encrypted_message, (nonce)(timestamp)(dh_key)(check)(data) );
FC_REFLECT( bts::bitchat::decrypted_message, (msg_type)(compression_format)(encryption_method)(data)(sig_time)(from_sig) )
FC_REFLECT( bts::bitchat::private_text_message, (msg) )
FC_REFLECT( bts::bitchat::private_email_message, (subject)(body)(attachments) )
FC_REFLECT( bts::bitchat::private_status_message, (status)(status_message) )
