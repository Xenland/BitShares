#include <bts/bitchat_message.hpp>
#include <fc/crypto/blowfish.hpp>
#include <fc/crypto/sha1.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

namespace bts
{
bitchat_message::bitchat_message()
:nonce(0),decrypted(false){}

mini_pow   bitchat_message::calculate_id()const
{
  std::vector<char> dat = fc::raw::pack( *this );
  return mini_pow_hash( dat.data(), dat.size() );
}

bitchat_message&  bitchat_message::sign( const fc::ecc::private_key& from )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->timestamp = fc::time_point::now();

    fc::sha256::encoder enc;
    fc::raw::pack( enc, static_cast<const content&>(*private_content) );

    private_content->from_sig = from.sign_compact( enc.result() );
    private_content->from     = from.get_public_key();
    return *this;
}
bitchat_message&  bitchat_message::body( const std::vector<char>& body  )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->body = body;
    return *this;
}

bitchat_message&  bitchat_message::reply_channel( const network::channel_id& c )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->reply_channel = c;
    return *this;
}

/**
 *  Encrypts the message using a random / newly generated one-time private
 *  key.
 */
void                        bitchat_message::encrypt(const fc::ecc::public_key& to)
{
    if( !private_content )
    {
       FC_THROW_EXCEPTION( exception, "attempt to encrypt message with no content" );
    }
    auto priv_dh_key = fc::ecc::private_key::generate(); 
    auto bf_key      = priv_dh_key.get_shared_secret( to );

    dh_key   = priv_dh_key.get_public_key();
    dh_check = fc::sha1::hash( bf_key )._hash[0];

    fc::blowfish bf;
    bf.start( (unsigned char*)&bf_key, sizeof(bf_key) );

    data = fc::raw::pack( *private_content );
    int extra = 8 - data.size() % 8;
    data.resize( data.size() + extra );
    bf.encrypt( (unsigned char*)data.data(), data.size() );

    decrypted = false;
    this->to = nullptr;
}

fc::optional<fc::ecc::private_key> bitchat_message::get_decryption_key()const
{
  return to;
}

/**
 *  
 *  @return true if the message could be decrypted with private_key k
 */
bool                        bitchat_message::decrypt( const fc::ecc::private_key& k )
{
  try 
  {
    FC_ASSERT( !decrypted );
    FC_ASSERT( data.size() > 0 );
    FC_ASSERT( data.size() % 8 == 0 );
    
    auto bf_key = k.get_shared_secret( dh_key );
    auto check  = fc::sha1::hash( bf_key )._hash[0];
    if( check != dh_check ) 
    {
      to = nullptr;
      return false;
    }
    
    fc::blowfish bf;
    bf.start( (unsigned char*)&bf_key, sizeof(bf_key) );
    
    bf.decrypt( (unsigned char*)data.data(), data.size() );
    
    private_content = fc::raw::unpack<signed_content>(data);

    fc::sha256::encoder enc;
    fc::raw::pack( enc, static_cast<const content&>(*private_content) );
    auto digest = enc.result();

    private_content->from = fc::ecc::public_key( private_content->from_sig, digest );
    
    to = k; 
    return decrypted = true;
  } FC_RETHROW_EXCEPTIONS( warn, "error decrypting message" );
}

bool                        bitchat_message::is_encrypted()const
{
  return !decrypted;
}

const bitchat_message::signed_content&       bitchat_message::get_content()const     
{
  if( !private_content ) 
  {
    FC_THROW_EXCEPTION( exception, "no private content set in message" );
  }
  return *private_content;
}

void                        bitchat_message::set_content( const signed_content& c)
{
  private_content = c;  
}

} // namespace bts
