#include <fc/crypto/sha512.hpp>
#include <fc/crypto/bigint.hpp>
#include <bts/mini_pow.hpp>
#include <algorithm>
#include <fc/log/logger.hpp>

namespace bts {

  mini_pow mini_pow_hash( const char* data, size_t len )
  {
      return mini_pow_hash( fc::sha512::hash( data, len ) );
  }

  const fc::bigint& max256()
  {
     static fc::bigint m = [](){ 
        fc::bigint tmp(1);
        tmp <<= (256);
        tmp -= 1;
        return tmp;
     }();
     return m;
  }

  mini_pow mini_pow_hash( const fc::sha512& h1 )
  {
      auto h2 = fc::sha512::hash(h1);
      auto h4 = fc::sha512::hash(h2);
      mini_pow p;
      fc::bigint  h3( (char*)&h2, sizeof(h2) );
      int32_t lz = 512 - h3.log2();
      memcpy( p.data, ((char*)&h4), sizeof(p) );
      p.data[0] = 255 - lz;
      return p;
  }
  fc::bigint     to_bigint( const mini_pow& p )
  {
      return fc::bigint( p.data, sizeof(p) );
  }
    
  mini_pow       to_mini_pow( const fc::bigint& i )
  {
      std::vector<char> bige = i;
      mini_pow p;
      if( bige.size() < sizeof(p) )
      {
          memset( p.data, 0, sizeof(p)-bige.size() );
      }
      memcpy( p.data + sizeof(p)-bige.size(), bige.data(), std::min<size_t>(bige.size(),sizeof(p)) );
      return p;
  }
  mini_pow   mini_pow_max()
  {
    static mini_pow max_pow = []()
        {
           mini_pow m;
           memset( m.data, 0xff, sizeof(mini_pow) );
           return m;
        }();
    return max_pow;
  }

  uint64_t mini_pow_difficulty( const mini_pow& pow )
  {
     uint8_t shift(pow.data[0]);
     if( shift <= 72 ) return -1;
     auto tmp = pow;
     tmp.data[0]=0; // leading byte must be 0 or the bigint will be
                    // interpreted as a negative number.
     fc::bigint bits( &tmp.data[0], sizeof(pow)-1 );
     bits <<= uint32_t(shift-72);
     return ((max256() / bits)).to_int64();
  }

}
