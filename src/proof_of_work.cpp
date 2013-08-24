#include <bts/proof_of_work.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/city.hpp>
#include <string.h>

#include <fc/io/raw.hpp>
#include <utility>
#include <fc/log/logger.hpp>

#define MB128 (128*1024*1024)

namespace bts  {

fc::uint128 proof_of_work( const fc::sha256& in)
{
   unsigned char* buf = new unsigned char[MB128];
   fc::uint128 out;
   try {
     out = proof_of_work( in, buf );
   } catch ( ... )
   {
    delete [] buf;
    throw;
   }
   delete[] buf;
   return out;
}


/**
 *  This proof-of-work is computationally difficult even for a single hash,
 *  but must be so to prevent optimizations to the required memory foot print.
 *
 *  The maximum level of parallelism achievable per GB of RAM is 8, and the highest
 *  end GPUs now have 4 GB of ram which means they could in theory support 32 
 *  parallel execution of this proof-of-work.     
 *
 *  On GPU's you only tend to get 1 instruction per 4 clock cycles in a single
 *  thread context.   Modern super-scalar CPU's can get more than 1 instruction
 *  per block and CityHash is specifically optomized to take advantage of this. 
 *  In addition to getting more done per-cycle, CPU's have close to 4x the clock
 *  frequency.
 *
 *  Based upon these characteristics alone, I estimate that a CPU can execute the
 *  serial portions of this algorithm at least 16x faster than a GPU which means
 *  that an 8-core CPU should easily compete with a 128 core GPU. Fortunately,
 *  a 128 core GPU would require 16 GB of RAM.  Note also, that most GPUs have 
 *  less than 128 'real' cores that are able to handle conditionals. 
 *
 *  Further more, GPU's are not well suited for branch misprediction and code
 *  must be optimized to avoid branches as much as possible.  
 *
 *  Lastly this algorithm takes advantage of a hardware instruction that is
 *  unlikely to be included in GPUs (Intel CRC32).  The lack of this hardware
 *  instruction alone is likely to give the CPU an order of magnitude advantage
 *  over the GPUs.
 */
fc::uint128 proof_of_work( const fc::sha256& iv, unsigned char* buffer_128m )
{
   auto key = fc::sha256(iv);
   const uint64_t  s = MB128/sizeof(uint64_t);
   uint64_t* buf = (uint64_t*)buffer_128m;
   memset( buffer_128m, 0, MB128/2 );
   fc::aes_encrypt( buffer_128m, MB128/2, (unsigned char*)&key, (unsigned char*)&iv,
                    buffer_128m + MB128/2 );
                    
   fc::aes_encrypt( buffer_128m + MB128/2, MB128/2, (unsigned char*)&iv, (unsigned char*)&key,
                    buffer_128m  );

   // use the last number generated in the sequence as the seed to
   // determine which numbers must be randomly swapped
   uint64_t data = (buf+s)[-1];
   for( uint32_t x = 0; x < 1024; ++x )
   {
      uint64_t d = data%s;
      uint64_t tmp = data ^ buf[d];
      std::swap( buf[tmp%s], buf[d] );
      data = tmp * (x+17);
   }

   return fc::city_hash_crc_128( (char*)buffer_128m, MB128 ); 
}


}  // namespace bts
