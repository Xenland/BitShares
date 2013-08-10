#include <bts/hd_wallet.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>

namespace bts {

  extended_public_key::extended_public_key()
  {
  }

  extended_public_key::~extended_public_key(){} 

  extended_public_key::extended_public_key( const fc::ecc::public_key& key, const fc::sha256& code )
  :pub_key(key),chain_code(code)
  {
  }


  extended_public_key extended_public_key::child( uint32_t child_idx )const
  { try {
      fc::sha512::encoder enc;
      fc::raw::pack( enc, pub_key );
      fc::raw::pack( enc, child_idx );

      fc::sha512 ikey  = enc.result();
      fc::sha256 ikey_left;
      fc::sha256 ikey_right;

      memcpy( (char*)&ikey_left, (char*)&ikey, sizeof(ikey_left) );
      memcpy( (char*)&ikey_right, ((char*)&ikey) + sizeof(ikey_left), sizeof(ikey_right) );

      extended_public_key child_key;
      child_key.chain_code = ikey_right;
      child_key.pub_key    = pub_key.add(ikey_left);

      return child_key;
  } FC_RETHROW_EXCEPTIONS( warn, "child index ${child_idx}", 
                                  ("exteneded_key",*this)("child_idx", child_idx ) ) }



  extended_private_key::extended_private_key( const fc::sha256& key, const fc::sha256& ccode )
  :priv_key(key),chain_code(ccode)
  {
  }

  extended_private_key::extended_private_key(){}

  extended_private_key extended_private_key::child( uint32_t child_idx, bool pub_derivation )const
  { try {
    extended_private_key child_key;

    fc::sha512::encoder enc;
    if( pub_derivation )
    {
      fc::raw::pack( enc, get_public_key() );
    }
    else
    {
      uint8_t pad = 0;
      fc::raw::pack( enc, pad );
      fc::raw::pack( enc, priv_key );
    }
    fc::raw::pack( enc, child_idx );
    fc::sha512 ikey = enc.result();

    fc::sha256 ikey_left;
    fc::sha256 ikey_right;

    memcpy( (char*)&ikey_left, (char*)&ikey, sizeof(ikey_left) );
    memcpy( (char*)&ikey_right, ((char*)&ikey) + sizeof(ikey_left), sizeof(ikey_right) );

    child_key.priv_key  = fc::ecc::private_key::generate_from_seed( priv_key, ikey_left ).get_secret();
    child_key.chain_code = ikey_right; 

    return child_key;
  } FC_RETHROW_EXCEPTIONS( warn, "child index ${child_idx}", 
                                  ("exteneded_key",*this)("child_idx", child_idx ) ) }

  extended_private_key::operator fc::ecc::private_key()const
  {
    return fc::ecc::private_key::regenerate( priv_key );
  }

  fc::ecc::public_key extended_private_key::get_public_key()const
  {
    return fc::ecc::private_key::regenerate( priv_key ).get_public_key();
  }
  
} // namespace bts
