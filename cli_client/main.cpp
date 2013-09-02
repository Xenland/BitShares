#include <bts/application.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/stdio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/fstream.hpp>
#include <fc/thread/thread.hpp>

#include <iostream>

using namespace bts;

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
     
     profile_ptr pro;
     try {
       pro = app->load_profile( "password" );
     } 
     catch ( fc::exception& e )
     {
       wlog( "${e}", ("e",e.to_detail_string() ) );
       app->create_profile( bts::profile_config(), "password" );
       pro = app->load_profile( "password" );
     }

     auto idents = pro->identities();
     ilog( "current idents: ${ids}", ("ids", idents ) );
     if( idents.size() == 0 )
     {
        ilog( "creating default identity..." );
        bts::identity new_id;
        new_id.label = "Daniel Larimer";
        new_id.bit_id = "bytemaster";
        new_id.mining_effort = 0.2;

        
        fc::optional<bitname::name_record> rec =  app->lookup_name( new_id.bit_id );
        if( rec )
        {
          ilog( "name is already in use: ${rec}", ("rec", *rec) );
          auto prv_key = pro->get_keychain().get_identity_key( new_id.bit_id );
          if( prv_key.get_public_key() != fc::ecc::public_key( rec->pub_key ) )
          {
              wlog( "name apparently belongs to someone else... I wouldn't recomend using this ID", ("rec", *rec) );
          }
        }
        pro->store_identity( new_id );
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
