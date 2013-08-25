#include <bts/proof_of_work.hpp>
#include <string.h>
#include <fc/io/stdio.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#define THREADS 5
#define ROUNDS  30
int main( int argc, char** argv )
{
   fc::sha256 in;
   if( argc >= 2 )
      in = fc::sha256::hash(argv[1],strlen(argv[1]));
   auto out = bts::proof_of_work( in );
   ilog( "out: ${out}", ("out",out));

   static fc::thread _threads[THREADS]; 
   fc::future<void> ready[THREADS];

   auto start = fc::time_point::now();
   for( uint32_t i = 0; i < THREADS; ++i )
   {
     ready[i] = _threads[i].async( [=]()
     {
       auto tin = in;
       unsigned char* tmp     = new unsigned char[32*1024*1024+512];
       for( int x = 0; x < ROUNDS; ++x )
       {
         ((uint16_t*)&tin)[i]++;
         bts::proof_of_work( in, tmp );
       }
       delete[] tmp;
     });
   }
   for( uint32_t i = 0; i < THREADS; ++i )
   {
    ready[i].wait();
   }
   auto end = fc::time_point::now();

   fc::cerr<<  ((ROUNDS*THREADS) / ((end-start).count() / 1000000.0))  <<  " hash / sec\n";


   return -1;
}
