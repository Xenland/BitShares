#include <bts/blockchain/transaction.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace blockchain {

   fc::sha256 transaction::digest()const
   {
      fc::sha256::encoder enc;
      fc::raw::pack( enc, *this );
      return enc.result();
   }

   std::unordered_set<address>             signed_transaction::get_signed_addresses()const
   {
       auto dig = digest(); 
       std::unordered_set<address> r;
       for( auto itr = sigs.begin(); itr != sigs.end(); ++itr )
       {
            r.insert( address(fc::ecc::public_key( *itr, dig )) );
       }
       return r;
   }

   uint160                                 signed_transaction::id()const
   {
      fc::sha512::encoder enc;
      fc::raw::pack( enc, *this );
      return small_hash( enc.result() );
   }

   void                                    signed_transaction::sign( const fc::ecc::private_key& k )
   {
      sigs.insert( k.sign_compact( digest() ) );
   }


} }
