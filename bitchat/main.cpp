#include <bts/network/server.hpp>
#include <bts/peer/peer_channel.hpp>
#include <bts/bitname/bitname_client.hpp>
#include <bts/bitchat/bitchat_client.hpp>
#include <bts/rpc/rpc_server.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/addressbook/contact.hpp>
#include <bts/extended_address.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/json.hpp>
#include <fc/io/fstream.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/stdio.hpp>
#include <fc/thread/thread.hpp>
#include <bts/network/upnp.hpp>

#include <iostream>
#include <sstream>

struct config
{
   fc::path                            data_dir;
   bts::network::server::config        server_config;
   bts::rpc::server::config            rpc_config;
   bts::bitname::client::config        bitname_config;
   std::vector<bts::bitchat::identity> ids;
   std::vector<bts::bitchat::contact>  contacts;
};
FC_REFLECT( config, (data_dir)(server_config)(rpc_config)(ids)(contacts)(bitname_config) )

class bitchat_del : public bts::bitchat::bitchat_delegate
{
   public:
     virtual void received_message( const std::string& msg, 
                                    const bts::bitchat::identity& to,
                                    const bts::bitchat::contact& from )
     {
        std::cout<<from.label<<": "<<msg<<"\n";     
     }
};

class bitname_test_del : public bts::bitname::client_delegate
{
   public:
    bitname_test_del()
    :current_name( fc::time_point::now().time_since_epoch().count() ){}

    bts::bitname::client* _client;
    /**
     *  Called when a new valid name trx is found 
     *  (either by mining or reported form the network).  
     */
    void bitname_header_pending( const bts::bitname::name_header& h )
    {
        ilog( "${h}", ("h",h) );
        if( h.name_hash == current_name )
        {
           current_name = fc::time_point::now().time_since_epoch().count();

          auto next_name = std::string(fc::time_point::now()); 
          ilog( "registering next name ${name}", ("name",next_name));
           _client->mine_name( next_name,
                                   fc::ecc::private_key::generate().get_public_key() );
        }
    };  

    /**
     *  Called anytime the head block is extended or replaced.
     */
    void bitname_block_added( const bts::bitname::name_block& h )
    {
        ilog( "block added:\n${s}", ("s", fc::json::to_pretty_string(h) ) );

        if( h.name_hash == current_name )
        {
           current_name = fc::time_point::now().time_since_epoch().count();

          auto next_name = std::string(fc::time_point::now()); 
          ilog( "registering next name ${name}", ("name",next_name));
           _client->mine_name( next_name,
                                   fc::ecc::private_key::generate().get_public_key() );
        }
    };  

    uint64_t current_name;
};


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
       out << fc::json::to_pretty_string( config() );
    }
    config cfg = fc::json::from_file( fc::path(argv[1]) ).as<config>();

    bts::addressbook::addressbook_ptr  address_book = std::make_shared<bts::addressbook::addressbook>();
    address_book->open( cfg.data_dir / "addressbook" );

    bts::network::server_ptr serv( std::make_shared<bts::network::server>() );    
    serv->configure( cfg.server_config );

    bts::peer::peer_channel_ptr    peer_ch        = std::make_shared<bts::peer::peer_channel>(serv);

    std::shared_ptr<bitchat_del>      chat_del       = std::make_shared<bitchat_del>();
    bts::bitchat::client_ptr       chat_cl        = std::make_shared<bts::bitchat::client>( peer_ch, address_book, chat_del.get() );

    /// provides synchronized accounts across all computers in the network
    bts::bitname::client_ptr       name_cl        = std::make_shared<bts::bitname::client>( peer_ch );
    std::shared_ptr<bitname_test_del> name_test_del  = std::make_shared<bitname_test_del>();
    name_test_del->_client = name_cl.get();
    name_cl->set_delegate( name_test_del.get() );
    name_cl->configure( cfg.bitname_config );


//    bts::blockchain::client_ptr    blockchain_cl  = std::make_shared<bts::blockchain::client>( peer_ch );

    /// enable local RPC queries of data on various channels
    bts::rpc::server_ptr rpc_serv = std::make_shared<bts::rpc::server>();
    rpc_serv->set_bitname_client( name_cl );
    rpc_serv->configure( cfg.rpc_config );

    auto  upnpserv = std::make_shared<bts::network::upnp_service>();
    upnpserv->map_port( cfg.server_config.port );

    if( argc >= 3 )
    {
      serv->connect_to( fc::ip::endpoint::from_string( argv[2] ) );
    }

    for( uint32_t i = 0; i < cfg.ids.size(); i++ )
    {
       chat_cl->add_identity( cfg.ids[i]);
    }
    for( uint32_t i = 0; i < cfg.contacts.size(); i++ )
    {
       chat_cl->add_contact( cfg.contacts[i]);
    }

    std::string line;
    fc::thread _cin("cin");
    std::cout<<"$] ";
    while( _cin.async([&](){ return !std::getline( std::cin, line ).eof(); } ).wait() ) 
    {
       std::stringstream ss(line);
       std::string cmd;
       ss >> cmd;

       if( cmd == "q" || cmd == "quit" )
       {
          break;
       }
       else if( cmd == "h" || cmd == "help" )
       {
          std::cout<<"q,quit                                             -  exit bitchat\n";
          std::cout<<"h,help                                             -  print this help message\n";
          std::cout<<"c,contact                   CONTACT_LABEL MSG_ADDR -  add a new contact / key pair\n";
          std::cout<<"n,new_id                    BITNAME                -  create a new identity \n";
          std::cout<<"i,ident                     BITNAME                -  switch identities \n";
          std::cout<<"s,send                      CONTACT_LABEL MESSAGE  -  send message to \n";
          std::cout<<"set_contact_pay_address     CONTACT_LABEL EXT_ADDR -  sets the address to send payments to contact \n";
          std::cout<<"pc,print_contact            BITNAME                \n";
          std::cout<<"l,list                                             -  list contacts & idents \n";
       }
       else if( cmd == "c" || cmd == "contact" )
       {
         std::string pub;
         bts::bitchat::contact new_contact;
         ss >> new_contact.label;
         ss >> pub;
         new_contact.key = bts::bitchat::address(pub);
         new_contact.send_channels.push_back(0);

         cfg.contacts.push_back( new_contact );

         chat_cl->add_contact( new_contact );
         fc::ofstream out( argv[1] );
         out << fc::json::to_pretty_string( cfg );
         std::cout << "created contact '"<<new_contact.label<<"' with address: "
                   << std::string( bts::bitchat::address( new_contact.key ) ) <<"\n";
       }
       else if( cmd == "n" || cmd == "new_id" )
       {
          bts::bitchat::identity id;
          ss >> id.label;
          id.key       = fc::ecc::private_key::generate();
          id.broadcast = fc::ecc::private_key::generate();
          id.recv_channels.push_back( 0 );
         
          chat_cl->add_identity( id );
          cfg.ids.push_back( id );
          fc::ofstream out( argv[1] );
          out << fc::json::to_pretty_string( cfg );
         
          std::cout << "created identity '"<<id.label<<"' with address: "<< std::string( bts::bitchat::address( id.key.get_public_key() ) ) <<"\n";
          name_cl->mine_name( id.label, id.key.get_public_key() );
       }
       else if( cmd == "set_contact_pay_address" )
       {
          std::string contact_bitname;
          std::string ext_pay_address;
          ss >> contact_bitname;
          ss >> ext_pay_address;

          auto contact_to_set = address_book->get_contact_by_bitname( contact_bitname );
          contact_to_set.send_trx_address = bts::extended_address(ext_pay_address);
          address_book->store_contact( contact_to_set );
       }
       else if( cmd == "pc" || cmd == "print_contact" )
       {
          std::string contact_bitname;
          ss >> contact_bitname;
          auto contact_to_print = address_book->get_contact_by_bitname( contact_bitname );
          std::cout << fc::json::to_pretty_string( contact_to_print ) <<"\n";
       }
       else if( cmd == "i" || cmd == "ident" )
       {
         std::string label;
         ss >> label;

       }
       else if( cmd == "s" || cmd == "send" )
       {
         if( !cfg.ids.size() )
         {
           std::cout<< "Please create an identity first\n";
           continue;
         }
         std::string label;
         ss >> label;
         std::string msg;
         std::getline( ss, msg );
         
         auto to_contact = chat_cl->get_contact( label );
         chat_cl->send_message( msg, to_contact, cfg.ids[0] );
       }
       else if( cmd == "l" || cmd == "list" )
       {
         std::cout<<fc::json::to_pretty_string( cfg )<<"\n";
         std::cout<<"Identities:\n";
         for( uint32_t i = 0; i < cfg.ids.size(); i++ )
         {
            std::cout<<"   "<<cfg.ids[i].label<<"  "<< std::string(bts::bitchat::address( cfg.ids[i].key.get_public_key() ))<<"\n";
         }
         std::cout<<"Contacts:\n";
         for( uint32_t i = 0; i < cfg.contacts.size(); i++ )
         {
            std::cout<<"   "<<cfg.contacts[i].label<<"  "<< std::string(bts::bitchat::address( cfg.contacts[i].key ))<<"\n";
         }
       }
       std::cout<<"$] ";
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
