#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/time.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/sha224.hpp>


namespace bts
{
    /**
     *  Define's the structure of messages broadcast on the
     *  bitmessage network.     
     */
    struct bitmessage
    {
        struct attachment
        {
            std::string        name;
            std::vector<char>  data;
        };

        struct content
        {
           fc::time_point                   timestamp;
           fc::ecc::public_key              to;
           std::vector<fc::ecc::public_key> cc;
           std::vector<fc::unsigned_int>    reply_channels;
           std::string                      subject;
           std::string                      body;
           std::vector<attachment>          attachments;
        };

        /** always serializes to a multiple of 8 bytes, this
         * data is encrypted via blowfish 
         */
        struct signed_content : public content
        {
          fc::ecc::compact_signature        from_sig; // applied to decrypted data
          fc::optional<fc::ecc::public_key> from; // not serialized
        };

        bitmessage();

        bitmessage&  to( const fc::ecc::public_key& to );
        bitmessage&  cc( const std::vector<fc::ecc::public_key>& cc ); 
        bitmessage&  subject( const std::string& subj  );
        bitmessage&  body( const std::string& bod  );
        bitmessage&  attach( const std::vector<attachment>& att );
        bitmessage&  reply_channel( uint32_t c );
        /**
         *  @brief the proof of work must be quick to verify relative to the
         *         time it took to generate because all nodes must validate 
         *         many messages.  
         */
        bitmessage&  do_proof_work( uint32_t d );
        bitmessage&  sign( const fc::ecc::private_key& from );

        void                        encrypt();
        fc::sha224                  calculate_id()const;
        bool                        decrypt( const fc::ecc::private_key& k );
        bool                        is_encrypted()const;
        const signed_content&       get_content()const;     
        void                        set_content(const signed_content& c);

        uint32_t                    nonce;
        fc::time_point              timestamp;
        fc::ecc::public_key         dh_key;
        fc::sha256                  dh_check;
        std::vector<char>           data;

        private:
        /** transient state of data vector */
        bool                          decrypted;
        fc::optional<signed_content>  private_content;

    };


}

FC_REFLECT( bts::bitmessage, (nonce)(timestamp)(dh_key)(dh_check)(data) )
FC_REFLECT( bts::bitmessage::content, (timestamp)(to)(cc)(reply_channels)(subject)(body)(attachments) )
FC_REFLECT_DERIVED( bts::bitmessage::signed_content, (bts::bitmessage::content), (from_sig) )
FC_REFLECT( bts::bitmessage::attachment, (name)(data) )

