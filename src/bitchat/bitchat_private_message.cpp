#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/crypto/blowfish.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace bitchat {

encrypted_message::encrypted_message()
:nonce(0),dh_check(0){}

mini_pow   encrypted_message::id()const
{
  fc::sha512::encoder enc;
  fc::raw::pack( enc, *this );
  return mini_pow_hash( enc.result() );
}

bool  encrypted_message::decrypt( const fc::ecc::private_key& with, decrypted_message& m )const
{
  try 
  {
    FC_ASSERT( data.size() > 0 );
    FC_ASSERT( data.size() % 8 == 0 );
    
    auto bf_key = with.get_shared_secret( dh_key );
    auto check  = fc::sha1::hash( bf_key )._hash[0];
    if( check != dh_check ) 
    {
      return false;
    }
    
    fc::blowfish bf;
    bf.start( (unsigned char*)&bf_key, sizeof(bf_key) );
    std::vector<char> tmp(data.size());
    bf.decrypt( (unsigned char*)data.data(), (unsigned char*)tmp.data(), data.size() );
    
    m = fc::raw::unpack<decrypted_message>(data);
    if( m.from_sig )
    {
        m.from_key = fc::ecc::public_key( *m.from_sig, m.digest() );
    } 
    m.decrypt_key = with; 
    return true;
  } FC_RETHROW_EXCEPTIONS( warn, "error decrypting message" );
}


fc::future<bool>  encrypted_message::do_proof_work( const mini_pow& tar_per_kb )
{
  return fc::async( [=](){ return false; } ); // TODO... implement do_proof_work
}



decrypted_message::decrypted_message()
:msg_type( unknown_msg ){}


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
    sig_time = fc::time_point::now();
    from_sig = from.sign_compact( digest() );
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
    auto bf_key      = priv_dh_key.get_shared_secret( to );

    em.dh_check = fc::sha1::hash( bf_key )._hash[0];

    fc::blowfish bf;
    bf.start( (unsigned char*)&bf_key, sizeof(bf_key) );

    em.data = fc::raw::pack( *this );
    // TODO: avoid extra dynamic memory alloc by pre-calc size
    int extra = 8 - em.data.size() % 8;
    em.data.resize( em.data.size() + extra );
    bf.encrypt( (unsigned char*)em.data.data(), em.data.size() );
   
   return em;
}


} } // namespace bts::bitchat
