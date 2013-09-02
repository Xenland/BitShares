#include <bts/application.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/stdio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/fstream.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>

int main( int argc, char** argv )
{
   try
   {
     if( argc < 2 )
     {
        fc::cerr<<"Usage "<<argv[0]<<" CONFIG  [BOOTSTRAP_HOST]\n"; 
        return -1;
     }

     if( !fc::exists( argv[1] ) )
     {
        fc::ofstream out( argv[1] );
        out << fc::json::to_pretty_string( bts::application_config() );
     }

     auto cfg = fc::json::from_file( fc::path(argv[1]) ).as<bts::application_config>();
     ilog( "config\n${cfg}", ("cfg", fc::json::to_pretty_string(cfg) ) );

     auto app = std::make_shared<bts::application>();
     app->configure( cfg );

     try {
       app->load_profile( "password" );
     } 
     catch ( fc::exception& e )
     {
       wlog( "${e}", ("e",e.to_detail_string() ) );
       app->create_profile( bts::profile_config(), "password" );
       app->load_profile( "password" );
     }


     std::string line;
     fc::thread _cin("cin");
     fc::cout<<"$] ";
     while( _cin.async([&](){ return !std::getline( std::cin, line ).eof(); } ).wait() ) 
     {
     }

     ilog( "shutting down" );
     return 0;
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}
