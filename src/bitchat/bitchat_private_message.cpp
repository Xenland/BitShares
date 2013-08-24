#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/raw.hpp>

#include <fc/log/logger.hpp>

namespace bts { namespace bitchat {

const private_message_type private_text_message::type = text_msg;
const private_message_type private_contact_request_message::type = contact_request_msg;
const private_message_type private_contact_auth_message::type = contact_auth_msg;
const private_message_type private_status_message::type = status_msg;

encrypted_message::encrypted_message()
:nonce(0),dh_check(0){}

fc::uint128   encrypted_message::id()const
{
  fc::sha512::encoder enc;
  fc::raw::pack( enc, *this );
  auto s512 = enc.result();
  return fc::city_hash128( (char*)&s512, sizeof(s512) );
}

bool  encrypted_message::decrypt( const fc::ecc::private_key& with, decrypted_message& m )const
{
  try 
  {
    FC_ASSERT( data.size() > 0 );
    FC_ASSERT( data.size() % 8 == 0 );
    
    auto aes_key = with.get_shared_secret( dh_key );
    uint32_t check  = fc::sha256::hash(fc::sha256::hash( aes_key ))._hash[0];
    if( check != dh_check )  
    { // 1 out of every 4 million msgs will have a false positive... 
      // and will cause the message to attempt decryption with bf, this
      // will cause failures later in the algo, but should not be
      // fatal to the program because in theory the unpacking
      // algorithm is secure against malicious data
      return false;
    }
    std::vector<char> tmp = fc::aes_decrypt( aes_key, data );
    m = fc::raw::unpack<decrypted_message>(tmp);
    if( m.from_sig )
    {
        try {
           m.from_key  = fc::ecc::public_key( *m.from_sig, m.digest() );
        } FC_RETHROW_EXCEPTIONS( warn, "error reconstructing public key ${msg}", ("msg",m) );
    } 
    m.decrypt_key = with; 
    return true;
  } FC_RETHROW_EXCEPTIONS( warn, "error decrypting message" );
}


/**
 * @param tar_per_kb... proof of work target per kb
 */
fc::future<bool>  encrypted_message::do_proof_work( uint64_t tar_per_kb )
{
  return fc::async( [=](){ return false; } ); // TODO... implement do_proof_work
}



decrypted_message::decrypted_message()
: msg_type( unknown_msg ),
 compression_format(no_compression),
 encryption_method(no_encryption) // no 'additional' encryption beyond the standard aes
 {}


fc::sha256   decrypted_message::digest()const
{
   fc::sha256::encoder enc;
   fc::raw::pack( enc, msg_type );
   fc::raw::pack( enc, data );
   fc::raw::pack( enc, sig_time );
   return enc.result();
}


decrypted_message&  decrypted_message::sign( const fc::ecc::private_key& from )
{
    sig_time  = fc::time_point::now();
    from_sig  = from.sign_compact( digest() );
    return *this;
}

/**
 *  Encrypts the message using a random / newly generated one-time private
 *  key.
 */
encrypted_message decrypted_message::encrypt(const fc::ecc::public_key& to)const
{
    encrypted_message em;
    auto priv_dh_key = fc::ecc::private_key::generate(); 
    em.dh_key        = priv_dh_key.get_public_key();
    auto aes_key      = priv_dh_key.get_shared_secret( to );

    em.dh_check = fc::sha256::hash(fc::sha256::hash( aes_key ))._hash[0];
    em.data = aes_encrypt( aes_key, fc::raw::pack(*this) );
   
    return em;
}


} } // namespace bts::bitchat
