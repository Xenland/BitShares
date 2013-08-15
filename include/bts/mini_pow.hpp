#pragma once
#include <fc/array.hpp>
#include <fc/crypto/bigint.hpp>

namespace fc { class sha512; }

namespace bts 
{
  /** 
   *  As of July 2013, the bitcoin network as a whole is only able
   *  to find a collision on 56 bits every 10 minutes.  And this level
   *  of work costs $2500 / 10 min.   Removing 16 bits of difficulty would still cost
   *  $0.50 which is far more than anyone would reasonably pay to broadcast
   *  a single message on the P2P network.
   *
   *  Given the POW hash function requires a lot of work to find a hash
   *  with 32 leading 0's, the amount of work required to find a collision
   *  on the remaining 64 bits would be enormus. 
   *  
   *  While this hash only has 80 bits, it can provide any level of security
   *  between 72 and 326 bits of hash based upon how much effort is put into
   *  finding the initial hash.  If you spend 1 hour calculating at 3K hash/sec,
   *  then the security is effectively 88 bits.  
   *
   *  The birthday attack does not apply equally to these 88 bits as it would 
   *  to a traditional hash because the difficulty of finding a 'pair' of hashes
   *  to test for equality would be 2 hours rather than milliseconds. An
   *  attacker would have to do a direct search for a specific hash and that
   *  would take the entire July 2013 Bitcoin network with the power of ASICs over
   *  80,000 years.  If CPU power doubles, then the same 1 hour of initial 
   *  calculation would still result in 80,000 years to break assuming the
   *  power of the Bitcoin network scaled as well.  It would take a bitcoin
   *  network 1000 times larger than the current network to bring this attack
   *  within the posability of our lifetime and in our use cases, this
   *  hash only need to be unique for a much shorter period.
   *
   *  To generate a collision would require over 1 trillion attempts and
   *  the difficulty of an attempt depends upon the first byte which
   *  specifies the number of bits of collision found on double SHA512()
   *
   *  The probability of finding a collision when the leading byte is 0,
   *  is equal to the probability of finding a collision on SHA256 + 72 bits
   *
   *  If the first byte is 255 then the hash security is essentially
   *  double SHA512 truncated to 72 bits. 
   *
   *  Use cases:
   *    Block Header Hash - 10,000+ CPUs crunching for 5 minutes to find
   *       a block with ~30+ leeding 0's and thus effectively 104+ bit. 
   *
   *    BitChat - propagating messages in a bandwidth effecient manner and
   *       identifying those messages with a unique hash.  The proof of work
   *       requirement on messages plus the relatively short life time means
   *       collisions are unlikely.
   *
   *    BitName - users are attempting to find a hash for proof of work and 
   *       spend at least 1 hour of CPU time name hash.  
   *
   **/
  typedef fc::array<char,10>  mini_pow;

  /**
   *  This hash function is designed to be secure, but
   *  fast to verify and produce as small of a hash value
   *  as possible while still ensuring 
   *
   *  The hash is calculated as 
   *
   *  pre   = SHA512( SHA512(data) )
   *  lzero = LOG2( pre )
   *  min_shift = max( 8, lzero - 8 )
   *  pre >> min_shift
   *  pre[0] = 255 - lzero;
   *  pre.trunc(10);
   *
   */
  mini_pow   mini_pow_hash( const char* data, size_t len );
  mini_pow   mini_pow_hash( const fc::sha512& seed );
  mini_pow   mini_pow_max();

  /**
   *  Converts the POW to a bigint so that operations may
   *  be performed on it.
   */
  fc::bigint     to_bigint( const mini_pow& p );
  mini_pow       to_mini_pow( const fc::bigint& i );

} // namespace bts
