#include <bts/network/server.hpp>
#include <bts/peer/peer_channel.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/json.hpp>
#include <fc/io/fstream.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>

struct config
{
   bts::network::server::config server_config;
};
FC_REFLECT( config, (server_config) )


int main( int argc, char** argv )
{
  try
  {
    if( argc != 2 )
    {
       fc::cerr<<"Usage "<<argv[0]<<" CONFIG\n"; 
       return -1;
    }
    if( !fc::exists( argv[1] ) )
    {
       fc::ofstream out( argv[1] );
       out << fc::json::to_pretty_string( config() );
    }
    config cfg = fc::json::from_file( fc::path(argv[1]) ).as<config>();

    bts::network::server_ptr serv( std::make_shared<bts::network::server>() );    
    serv->configure( cfg.server_config );

    bts::peer::peer_channel_ptr peer_ch = std::make_shared<bts::peer::peer_channel>(serv);



    return 0;
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}
