#include <bts/proof_of_work.hpp>
#include <string.h>
#include <fc/io/stdio.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>
#include <SFMT.h>

#include <fc/io/raw.hpp>
#include <fc/crypto/ripemd160.hpp>
#include <utility>
#include <fc/log/logger.hpp>
#include <fc/crypto/city.hpp>
#include <fc/crypto/sha512.hpp>
#include <fc/crypto/sha256.hpp>
#include <fc/crypto/aes.hpp>

#define BLOCK_SIZE (128) // bytes

bts::pow_hash test_proof_of_work( const fc::sha256& seed, unsigned char* buffer, size_t size )
{
   auto key = fc::sha256(seed);
   auto iv  = fc::city_hash128((char*)&seed,sizeof(seed));

   sfmt_t gen;
   sfmt_init_by_array( &gen, (uint32_t*)&iv, sizeof(iv)/sizeof(uint32_t) );
   sfmt_fill_array64( &gen, (uint64_t*)buffer, size/sizeof(uint64_t) );

   uint64_t* read_pos  = (uint64_t*)(buffer);
   uint64_t* write_pos = (uint64_t*)(buffer+32);

   fc::aes_encoder enc( key, iv );
   auto loop_count = size / (2*BLOCK_SIZE);
   auto half_loop = loop_count / 4;
   uint32_t count = 0;
   for( uint32_t i = 0; i < loop_count; ++i )
   {
      uint64_t wrote = enc.encode( (char*)read_pos, BLOCK_SIZE, (char*)write_pos ); 
      read_pos  =  (uint64_t*)( buffer + (write_pos[0]) % ( size - BLOCK_SIZE ) );
      if( i > half_loop ) i += (write_pos[2]%17)-8;
      write_pos =  (uint64_t*)( buffer + (write_pos[1]) % ( size - BLOCK_SIZE ) );
      ++count;
   }
   
   //ilog( "count: ${c}", ("c",count));
   auto midstate =  fc::city_hash_crc_256( (char*)buffer, size ); 
   return fc::ripemd160::hash((char*)&midstate, sizeof(midstate) );
}


#define THREADS 1
#define ROUNDS  1000 
#define BUF_SIZE (64*1024)
int main( int argc, char** argv )
{
   fc::sha256 in;
   if( argc >= 2 )
      in = fc::sha256::hash(argv[1],strlen(argv[1]));
   auto out = bts::proof_of_work( in );
   ilog( "out: ${out}", ("out",out));

   static fc::thread _threads[THREADS]; 

   size_t buf_size = BUF_SIZE;
   for( uint32_t i = 0; i < 10; ++i )
   {
     // fc::future<void> ready[THREADS];
      auto start = fc::time_point::now();
      //for( uint32_t i = 0; i < THREADS; ++i )
      {
      //  ready[i] = _threads[i].async( [=]()
      //  {
          auto tin = in;
          unsigned char* tmp     = new unsigned char[32*1024*1024+512];
          for( int x = 0; x < ROUNDS; ++x )
          {
            ((uint16_t*)&tin)[i]++;
            test_proof_of_work( tin, tmp, buf_size );
          }
          delete[] tmp;
      //  });
      }
      /*
      for( uint32_t i = 0; i < THREADS; ++i )
      {
       ready[i].wait();
      }
      */
      auto end = fc::time_point::now();
      fc::cerr<< buf_size <<"  "<<((ROUNDS*THREADS) / ((end-start).count() / 1000000.0))  <<  " hash / sec\n";
      buf_size *= 2;
   }


   return -1;
}
