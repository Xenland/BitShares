#include <bts/bitmessage.hpp>
#include <fc/crypto/blowfish.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/raw.hpp>

namespace bts
{
bitmessage::bitmessage()
:nonce(0),decrypted(false){}

fc::sha224   bitmessage::calculate_id()const
{
  fc::sha224::encoder enc;
  fc::raw::pack( enc, *this );
  return enc.result();
}

bitmessage&  bitmessage::sign( const fc::ecc::private_key& from )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->timestamp = fc::time_point::now();

    fc::sha256::encoder enc;
    fc::raw::pack( enc, static_cast<const content&>(*private_content) );

    private_content->from_sig = from.sign_compact( enc.result() );
    private_content->from     = from.get_public_key();
    return *this;
}

bitmessage&  bitmessage::to( const fc::ecc::public_key& to )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->to = to;
    return *this;
}

bitmessage&  bitmessage::cc( const std::vector<fc::ecc::public_key>& cc )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->cc = cc;
    return *this;
}
 
bitmessage&  bitmessage::subject( const std::string& subj  )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->subject = subj;
    return *this;
}

bitmessage&  bitmessage::body( const std::string& bod  )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->body = bod;
    return *this;
}

bitmessage&  bitmessage::attach( const std::vector<attachment>& att )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->attachments = att;
    return *this;
}


bitmessage&  bitmessage::reply_channel( uint32_t c )
{
    if( !private_content ) { private_content = signed_content(); }
    private_content->reply_channels.push_back(c);
    return *this;
}

/**
 *  Encrypts the message using a random / newly generated one-time private
 *  key.
 */
void                        bitmessage::encrypt()
{
    if( !private_content )
    {
       FC_THROW_EXCEPTION( exception, "attempt to encrypt message with no content" );
    }
    auto priv_dh_key = fc::ecc::private_key::generate(); 
    auto bf_key      = priv_dh_key.get_shared_secret( private_content->to );

    dh_key   = priv_dh_key.get_public_key();
    dh_check = fc::sha256::hash( bf_key );

    fc::blowfish bf;
    bf.start( (unsigned char*)&bf_key, sizeof(bf_key) );

    data = fc::raw::pack( *private_content );
    int extra = 8 - data.size() % 8;
    data.resize( data.size() + extra );
    bf.encrypt( (unsigned char*)data.data(), data.size() );

    decrypted = false;
}

/**
 *  
 *  @return true if the message could be decrypted with private_key k
 */
bool                        bitmessage::decrypt( const fc::ecc::private_key& k )
{
  try 
  {
    FC_ASSERT( !decrypted );
    FC_ASSERT( data.size() > 0 );
    FC_ASSERT( data.size() % 8 == 0 );
    
    auto bf_key = k.get_shared_secret( dh_key );
    auto check  = fc::sha256::hash( bf_key );
    if( check != dh_check ) 
    {
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

    return decrypted = true;
  } FC_RETHROW_EXCEPTIONS( warn, "error decrypting message" );
}

bool                        bitmessage::is_encrypted()const
{
  return decrypted;
}

const bitmessage::signed_content&       bitmessage::get_content()const     
{
  if( !private_content ) 
  {
    FC_THROW_EXCEPTION( exception, "no private content set in message" );
  }
  return *private_content;
}

void                        bitmessage::set_content( const signed_content& c)
{
  private_content = c;  
}

} // namespace bts
