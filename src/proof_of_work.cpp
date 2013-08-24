#include <bts/proof_of_work.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/aes.hpp>
#include <fc/crypto/salsa20.hpp>
#include <fc/crypto/city.hpp>
#include <string.h>

#include <fc/io/raw.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <utility>
#include <fc/log/logger.hpp>

#define MB128 (128*1024*1024)

namespace bts  {

pow_hash proof_of_work( const fc::sha256& in)
{
   unsigned char* buf = new unsigned char[MB128];
   pow_hash out;
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
pow_hash proof_of_work( const fc::sha256& iv, unsigned char* buffer_128m )
{
   auto key = fc::sha256(iv);
   const uint64_t  s = MB128/sizeof(uint64_t);
   uint64_t* buf = (uint64_t*)buffer_128m;
   memset( buffer_128m, 0, MB128/2 );
   
   fc::aes_encrypt( buffer_128m, MB128/2, (unsigned char*)&key, (unsigned char*)&iv,
                    buffer_128m + MB128/2 );
   
   uint64_t offset = buf[s-1] % ((MB128/2)-1024);
   fc::sha512 new_key = fc::sha512::hash( (char*)(buffer_128m + offset + MB128/2), 1024 );
                    
   fc::aes_encrypt( buffer_128m + MB128/2, MB128/2, (unsigned char*)&new_key,
                                                    ((unsigned char*)&new_key) + 32,
                    buffer_128m  );

   auto midstate =  fc::city_hash_crc_256( (char*)buffer_128m, MB128 ); 
   return fc::ripemd160::hash((char*)&midstate, sizeof(midstate) );
}


}  // namespace bts
