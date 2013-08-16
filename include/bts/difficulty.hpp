#pragma once
#include <fc/crypto/bigint.hpp>
#include <fc/crypto/sha224.hpp>

namespace bts 
{
  
  /**
   *  Calculates the difficulty of hash as 
   *
   *  (max224() / hash) >> 24 
   */
  uint64_t difficulty( const fc::sha224& hash_value );

  /** return 2^224 -1 */
  const fc::bigint&  max224();

} // namespace bts
