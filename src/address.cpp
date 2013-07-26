#include <bts/address.hpp>
#include <fc/crypto/base58.hpp>

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
   }

   address::address( const fc::ecc::public_key& pub )
   {
       auto dat      = pub.serialize();
       auto dat_hash = fc::sha256::hash( dat.data, sizeof(dat) );
       ((char*)&dat_hash)[0] &= 0x1f; // set the first 4 bits to 0
       ((char*)&dat_hash)[0] |= 0x10; // set the first 4 bits to 0
       auto check = fc::sha256::hash( (char*)&dat_hash, 16 );
       memcpy( &addr.data[4*4], (char*)&check, 4 );
       memcpy( addr.data, (char*)&dat_hash, sizeof(addr) );
   }

   /**
    *  Checks the address to verify it has a 
    *  valid checksum and prefix.
    */
   bool address::is_valid()const
   {
       if( (addr.data[0] & 0xf0)  != 0x10 ) return false; 
       auto check = fc::sha256::hash( addr.data, 16 );
       return memcmp(&addr.data[4*4], &check, 4 ) == 0;
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
