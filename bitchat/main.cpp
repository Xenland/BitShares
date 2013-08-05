#include <bts/network/server.hpp>
#include <bts/peer/peer_channel.hpp>
#include <bts/bitchat/bitchat_client.hpp>
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

class bitchat_del : public bts::bitchat::bitchat_delegate
{
   public:
     virtual void received_message( const std::string& msg, 
                                    const bts::bitchat::identity& to,
                                    const bts::bitchat::contact& from )
     {
     
     }
};


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

    std::shared_ptr<bitchat_del> chat_del = std::make_shared<bitchat_del>();

    bts::bitchat::client_ptr chat_cl = std::make_shared<bts::bitchat::client>( peer_ch, chat_del.get() );

    return 0;
  } 
  catch ( const fc::exception& e )
  {
     elog( "${e}", ("e", e.to_detail_string() ) );
  }
  return -1;
}
