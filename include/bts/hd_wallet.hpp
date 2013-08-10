#pragma once
#include <fc/crypto/elliptic.hpp>

namespace bts {

  class extended_public_key
  {
     public:
        extended_public_key();
        ~extended_public_key();

        extended_public_key( const fc::ecc::public_key& key, const fc::sha256& code );

        extended_public_key child( uint32_t c )const;

        operator fc::ecc::public_key()const { return pub_key; }

        fc::ecc::public_key pub_key;
        fc::sha256          chain_code;
  };

  class extended_private_key
  {
     public:
        extended_private_key( const fc::sha256& key, const fc::sha256& chain_code );
        extended_private_key();

        /** @param pub_derivation - if true, then extended_public_key can be used
         *      to calculate child keys, otherwise the extended_private_key is
         *      required to calculate all children.
         */
        extended_private_key child( uint32_t c, bool pub_derivation = false )const;

        operator fc::ecc::private_key()const;
        fc::ecc::public_key get_public_key()const;
       
        fc::sha256          priv_key;
        fc::sha256          chain_code;
  };



} // bts
#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::extended_public_key,  (pub_key)(chain_code)  )
FC_REFLECT( bts::extended_private_key, (priv_key)(chain_code) )
