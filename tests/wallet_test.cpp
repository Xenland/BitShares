#include "../src/wallet.hpp"
#include <fc/io/stdio.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <assert.h>

int main( int argc, char** argv )
{
   if( argc < 3 )
   {
      fc::cerr<<"Usage: "<<argv[0]<<" WALLET PASSWORD\n";
      return -1;
   }
   try
   {
      wallet w;
      if( fc::exists( fc::path( argv[1] ) ) )
      {
          w.load( fc::path(argv[1]), argv[2] );
          assert( !w.is_locked() );
          auto r = w.get_addresses();
          for( auto i = r.begin(); i != r.end(); ++i )
             ilog( "loaded address: ${addr}", ("addr", *i) );
      }
      else
      {
          auto r = w.reserve( 10 );
          w.save( fc::path(argv[1]), argv[2] );
          for( auto i = r.begin(); i != r.end(); ++i )
             ilog( "reserved address: ${addr}", ("addr", *i) );
      }
   } 
   catch( const fc::exception& e )
   {
      fc::cerr<<e.to_detail_string();
   }

   return 0;
}
