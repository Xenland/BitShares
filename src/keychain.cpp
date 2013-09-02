#include <bts/keychain.hpp>
#include <bts/proof_of_work.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/thread/thread.hpp>

#include <fc/log/logger.hpp>

namespace bts {

  keychain::keychain(){}

  /**
   *  This method will take several minutes to run and is designed to
   *  make rainbow tables difficult to calculate.
   */
  fc::sha512 keychain::stretch_seed( const fc::sha512& seed )
  {
      fc::thread t("stretch_seed");
      return t.async( [=]() {
          fc::sha512 last = seed;
          ilog( "stretchign seed" );
          for( uint32_t i = 0; i < 10; ++i )
          {
              ilog( ".\r" );
              auto p = proof_of_work( fc::sha256::hash( (char*)&last, sizeof(last)) );  
              last = fc::sha512::hash( (char*)&p, sizeof(p) );
          }
          return last; 
      } ).wait();
  }

  void              keychain::set_seed( const fc::sha512& stretched_seed )
  {
    ext_priv_key = extended_private_key(stretched_seed);
  }

  fc::sha512        keychain::get_seed()const
  {
    static_assert( sizeof(*this) == sizeof( fc::sha512), "make sure there is no funny packing going on" );
    fc::sha512 seed;
    memcpy( (char*)&seed, (char*)this, sizeof(seed) );
    return seed;
  }
  extended_private_key  keychain::get_identity_key( const std::string& ident )
  {
    return ext_priv_key.child( fc::city_hash64(ident.c_str(),ident.size()),false );
  }

  extended_private_key  keychain::get_private_account( const std::string& ident, uint32_t i )
  {
    return get_identity_key(ident).child( i, false );
  }

  extended_public_key   keychain::get_public_account( const std::string& ident, uint32_t i )
  {
    auto priv_acnt = get_private_account(ident,i);
    return extended_public_key( priv_acnt.get_public_key(), priv_acnt.chain_code );
  }

  extended_public_key   keychain::get_public_trx( const std::string& ident, uint32_t account, uint32_t trx )
  {
    auto r = get_public_account( ident, account ).child( trx );
//    ilog( "ext pub trx: ${account}/${trx} => ${epk}", ("account",account)("trx",trx)("epk",r) );
    return r;
  }

  fc::ecc::public_key   keychain::get_public_trx_address( const std::string& ident, uint32_t account, uint32_t trx, uint32_t addr )
  {
    return get_public_trx( ident, account, trx ).child(addr);
  }

  extended_private_key  keychain::get_private_trx( const std::string& ident, uint32_t account, uint32_t trx )
  {
    auto r =  get_private_account( ident, account ).child( trx, true/*pub deriv*/ );
 //   ilog( "ext priv trx: ${account}/${trx} => ${epk}", ("account",account)("trx",trx)("epk",r) );
    return r;
  }

  fc::ecc::private_key  keychain::get_private_trx_address( const std::string& ident, uint32_t account, uint32_t trx, uint32_t addr )
  {
    return get_private_trx( ident, account, trx ).child(addr, true /*pub deriv*/);
  }

  
} // namespace bts
