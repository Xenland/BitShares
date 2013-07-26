#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/time.hpp>
#include <fc/optional.hpp>
#include <bts/mini_pow.hpp>
#include <bts/network/channel_id.hpp>

namespace bts {

    enum message_types 
    {
       data_msg      = 1, ///< a message encrypted to unknown receip (sent in reponse to get_msg)
       inventory_msg = 2, ///< publishes known invintory
       get_inventory = 3, ///< fetch known invintory
       get_data_msg  = 4  ///< sent to request an inventory item
    };


    /**
     *  Define's the structure of messages broadcast on the
     *  bitchat_message network.     
     */
    struct bitchat_message
    {
        enum type_enum { type = message_types::data_msg };

        struct content
        {
           network::channel_id  reply_channel;
           std::vector<char>    body;
        };

        /** always serializes to a multiple of 8 bytes, this
         * data is encrypted via blowfish 
         */
        struct signed_content : public content
        {
          fc::time_point                    timestamp;// certifies signature
          fc::ecc::compact_signature        from_sig; // applied to decrypted data
          fc::optional<fc::ecc::public_key> from; // not serialized
        };

        bitchat_message();

        bitchat_message&            body( const std::vector<char>& dat  );
        bitchat_message&            reply_channel( const network::channel_id& c );

        /**
         *  @brief the proof of work must be quick to verify relative to the
         *         time it took to generate because all nodes must validate 
         *         many messages.  
         */
        bitchat_message&            do_proof_work( const mini_pow& tar_per_kb );
        bitchat_message&            sign( const fc::ecc::private_key& from );

        void                        encrypt( const fc::ecc::public_key& to );
        mini_pow                    calculate_id()const;
        bool                        decrypt( const fc::ecc::private_key& k );
        bool                        is_encrypted()const;
        const signed_content&       get_content()const;     
        void                        set_content(const signed_content& c);

        fc::optional<fc::ecc::private_key> get_decryption_key()const;

        uint16_t                    nonce; ///< increment timestamp after 63K tests
        fc::time_point              timestamp;
        fc::ecc::public_key         dh_key;
        uint32_t                    dh_check;
        std::vector<char>           data;

        private:
        /** transient state of data vector */
        bool                                decrypted;
        fc::optional<signed_content>        private_content;
        fc::optional<fc::ecc::private_key>  to; // key that decrypted the message

    };


    // other protocol messages... not encapsulated by data_message
    struct inv_message 
    {
        enum msg_type_enum { type = message_types::inventory_msg };
        std::unordered_set<mini_pow>  items;
    };

    struct get_data_message
    {
        enum msg_type_enum { type = message_types::get_data_msg };
        std::unordered_set<mini_pow>  items;
    };





    /** content of bitchat_message data */
    enum data_message_types
    {
       bitchat_text_msg = 1
    };
    struct data_message
    {
        enum msg_type_enum { type = bitchat_text_msg };
        data_message(const std::string& s = "")
        :msg_type( type ),msg(s){}
        fc::unsigned_int msg_type;
        std::string      msg;
    };

}  // namespace btc

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::bitchat_message::content,        (reply_channel)(body) )
FC_REFLECT_DERIVED( bts::bitchat_message::signed_content, (bts::bitchat_message::content), (timestamp)(from_sig) )
FC_REFLECT( bts::bitchat_message,  (nonce)(timestamp)(dh_key)(dh_check)(data) )
FC_REFLECT( bts::data_message, (msg_type)(msg) );
FC_REFLECT( bts::inv_message, (items) )
FC_REFLECT( bts::get_data_message, (items) )
