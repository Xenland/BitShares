#include <bts/hd_wallet.hpp>
#include <bts/proof_of_work.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>

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

  extended_private_key::extended_private_key( const fc::sha512& seed )
  {
    memcpy( (char*)&priv_key, (char*)&seed, sizeof(priv_key) );
    memcpy( (char*)&chain_code, ((char*)&seed) + sizeof(priv_key), sizeof(priv_key) );
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


  /**
   *  This method will take several minutes to run and is designed to
   *  make rainbow tables difficult to calculate.
   */
  fc::sha512 hd_wallet::stretch_seed( const fc::sha512& seed )
  {
      fc::thread t("stretch_seed");
      return t.async( [=]() {
          fc::sha512 last = seed;
          for( uint32_t i = 0; i < 100; ++i )
          {
              auto p = proof_of_work( fc::sha256::hash( (char*)&last, sizeof(last)) );  
              last = fc::sha512::hash( (char*)&p, sizeof(p) );
          }
          return last; 
      } ).wait();
  }

  void              hd_wallet::set_seed( const fc::sha512& stretched_seed )
  {
    ext_priv_key = extended_private_key(stretched_seed);
  }

  fc::sha512        hd_wallet::get_seed()const
  {
    static_assert( sizeof(*this) == sizeof( fc::sha512), "make sure there is no funny packing going on" );
    fc::sha512 seed;
    memcpy( (char*)&seed, (char*)this, sizeof(seed) );
    return seed;
  }

  extended_private_key  hd_wallet::get_private_account( uint32_t i )
  {
    return ext_priv_key.child( i, false );
  }

  extended_public_key   hd_wallet::get_public_account( uint32_t i )
  {
    auto priv_acnt = get_private_account(i);
    return extended_public_key( priv_acnt.get_public_key(), priv_acnt.chain_code );
  }

  extended_public_key   hd_wallet::get_public_trx( uint32_t account, uint32_t trx )
  {
    return get_public_account( account ).child( trx );
  }

  fc::ecc::public_key   hd_wallet::get_public_trx_address( uint32_t account, uint32_t trx, uint32_t addr )
  {
    return get_public_trx( account, trx ).child(addr);
  }

  extended_private_key  hd_wallet::get_private_trx( uint32_t account, uint32_t trx )
  {
    return get_private_account( account ).child( trx );
  }

  fc::ecc::private_key  hd_wallet::get_private_trx_address( uint32_t account, uint32_t trx, uint32_t addr )
  {
    return get_private_trx( account, trx ).child(addr);
  }

  
} // namespace bts
