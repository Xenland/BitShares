#include <bts/address.hpp>
#include <bts/small_hash.hpp>
#include <fc/crypto/base58.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <fc/exception/exception.hpp>

namespace bts
{
   address::address()
   {
    memset( addr.data, 0, sizeof(addr.data) );
   }
   address::address( const std::string& base58str )
   {
      std::vector<char> v = fc::from_base58( fc::string(base58str) );
      if( v.size() )
         memcpy( addr.data, v.data(), std::min<size_t>( v.size(), sizeof(addr) ) );

      if( !is_valid() )
      {
         FC_THROW_EXCEPTION( exception, "invalid address ${a}", ("a", base58str) );  
      }
   }

   address::address( const fc::ecc::public_key& pub )
   {
       auto dat      = pub.serialize();
       auto dat_hash = small_hash(dat.data, sizeof(dat) );
       // set the version number
       ((char*)&dat_hash)[0] &= 0x0f; // set first 4 bits to 0 
       ((char*)&dat_hash)[0] |= 0x10; // set the first 4 bits to 0001
       auto check = fc::ripemd160::hash( (char*)&dat_hash, 16 );
       memcpy( addr.data, (char*)&dat_hash, sizeof(addr) );
       memcpy( &addr.data[16], (char*)&check, 4 );
   }

   /**
    *  Checks the address to verify it has a 
    *  valid checksum and prefix.
    */
   bool address::is_valid()const
   {
       if( (addr.data[0] & 0xf0) != 0x10 ) // version 1
       {
          FC_THROW_EXCEPTION( exception, "invalid address version" );
          return false; 
       }
       auto check = fc::ripemd160::hash( addr.data, 16 );
       return memcmp(&addr.data[16], &check, 4 ) == 0;
   }

   address::operator std::string()const
   {
        return fc::to_base58( addr.data, sizeof(addr) );
   }
} // namespace bts


namespace fc 
{ 
   void to_variant( const bts::address& var,  variant& vo )
   {
        vo = std::string(var);
   }
   void from_variant( const variant& var,  bts::address& vo )
   {
        vo = bts::address( var.as_string() );
   }
}
