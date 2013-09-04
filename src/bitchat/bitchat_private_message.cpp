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
const private_message_type private_email_message::type = email_msg;
const private_message_type private_contact_request_message::type = contact_request_msg;
const private_message_type private_contact_auth_message::type = contact_auth_msg;
const private_message_type private_status_message::type = status_msg;

encrypted_message::encrypted_message()
:nonce(0){}

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

    auto check_hash = fc::sha512::hash( aes_key );
    fc::ripemd160::encoder enc;
    fc::raw::pack( enc, check_hash );
    fc::raw::pack( enc, data       );
    if( check != enc.result() )
    {
      return false;
    }
    wlog( "we passed checksum test... unpack message.." );

    std::vector<char> tmp = fc::aes_decrypt( aes_key, data );
    m = fc::raw::unpack<decrypted_message>(tmp);
    ilog( "type: ${t}", ("t",uint64_t(m.msg_type)) );
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
: msg_type( unknown_msg )
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
    em.data = aes_encrypt( aes_key, fc::raw::pack(*this) );


    auto check_hash = fc::sha512::hash( aes_key );
    fc::ripemd160::encoder enc;
    fc::raw::pack( enc, check_hash     );
    fc::raw::pack( enc, em.data   );
    em.check = enc.result();
   
    return em;
}


} } // namespace bts::bitchat
