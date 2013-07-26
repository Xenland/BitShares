#include <bts/proof_of_work.hpp>
#include <string.h>
#include <fc/io/stdio.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

int main( int argc, char** argv )
{
   fc::sha256 in;
   if( argc >= 2 )
      in = fc::sha256::hash(argv[1],strlen(argv[1]));

   static fc::thread _threads[8]; 
   fc::future<void> ready[8];

   auto start = fc::time_point::now();
   for( uint32_t i = 0; i < 8; ++i )
   {
     ready[i] = _threads[i].async( [=]()
     {
       auto tin = in;
       unsigned char* tmp     = new unsigned char[128*1024*1024+512];
       for( int x = 0; x < 25; ++x )
       {
         ((uint16_t*)&tin)[i]++;
         bts::proof_of_work( in, tmp );
       }
       delete[] tmp;
     });
   }
   for( uint32_t i = 0; i < 8; ++i )
   {
    ready[i].wait();
   }
   auto end = fc::time_point::now();

   fc::cerr<<  (200.0 / ((end-start).count() / 1000000.0))  <<  " hash / sec\n";

   unsigned char* tmp     = new unsigned char[128*1024*1024+512];
   auto out = bts::proof_of_work( in, tmp );
   ilog( "out: ${out}", ("out",out));

   delete[] tmp;
   return -1;
}
