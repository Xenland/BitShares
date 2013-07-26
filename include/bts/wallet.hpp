#pragma once 
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>

namespace bts
{
  /**
   *  @class wallet
   *  @brief A deterministic wallet that generates a family of public/private keys.
   *
   *  A wallet can deterministicaly generate a whole family of public / private keys
   *  given an initial secret value.  If only the master public key is given, then
   *  all other public keys could be generated, however, the private keys could not
   *  be generated.  This makes it easy to have read-only wallets that do not
   *  require access to the secret.
   *
   *  This wallet class only conserns itself with the generation of the deterministic
   *  keys and does not implement any storage system or caching. 
   */
  class wallet
  {
    public:
       wallet();
       ~wallet();

       /**
        *  The seed should be the hash of a long pass phrase and not something that
        *  would end up in a dictionary attack.
        */
       void                 set_seed( const fc::sha256& s );
       
       /**
        *  Used to generate public keys without any private keys.
        */
       void                 set_master_public_key( const fc::ecc::public_key& k );
       fc::ecc::public_key  get_master_public_key()const;

       /**
        *  @throws if no seed has been set.
        */
       fc::ecc::private_key get_master_private_key()const;

       /**
        *  @throws if set_seed() or set_master_public_key() has not been called
        */
       fc::ecc::public_key  get_public_key( uint32_t index );

       /**
        *  @throws if no seed has been set.
        */
       fc::ecc::private_key get_private_key( uint32_t index );

    private:
       fc::sha256                           _stretched_seed;
       fc::optional<fc::ecc::private_key>   _master_priv;
       fc::optional<fc::ecc::public_key>    _master_pub;
  };

}
