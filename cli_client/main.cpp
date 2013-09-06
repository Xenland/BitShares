#include <bts/application.hpp>
#include <fc/io/json.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/stdio.hpp>
#include <fc/log/logger.hpp>
#include <fc/io/fstream.hpp>
#include <fc/thread/thread.hpp>
#include <sstream>
#include <fc/io/json.hpp>
#include <iostream>

using namespace bts;

void create_identity( const std::shared_ptr<bts::application>& app )
{
    bts::identity new_id;

    std::cout<<"\n Create new Identity\n";
    std::cout<<"Label: ";
    std::cin >> new_id.label;
    std::cout<<"BitID: ";
    std::cin >> new_id.bit_id;

    new_id.mining_effort = 0.2;
    auto pro = app->get_profile();
    
    fc::optional<bitname::name_record> rec =  app->lookup_name( new_id.bit_id );
    if( rec )
    {
      ilog( "name is already in use: ${rec}", ("rec", *rec) );
      auto prv_key = pro->get_keychain().get_identity_key( new_id.bit_id );
      if( prv_key.get_public_key() != fc::ecc::public_key( rec->pub_key ) )
      {
          wlog( "name apparently belongs to someone else... I wouldn't recomend using this ID", ("rec", *rec) );
      }
      else
      {
        ilog( "name belongs to me, so we are ok..." );
      }
    }
    pro->store_identity( new_id );
}


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

     if( argc >= 3 )
     {
        app->add_node( fc::ip::endpoint::from_string( argv[2] ) );
     }
     
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
        create_identity( app );
        idents = pro->identities();
     }

     fc::ecc::private_key my_priv_key = pro->get_keychain().get_identity_key( idents[0].bit_id );



     std::string line;
     fc::thread _cin("cin");
     fc::cout<<"$] ";
     while( _cin.async([&](){ return !std::getline( std::cin, line ).eof(); } ).wait() ) 
     {
       std::stringstream ss(line);
       std::string cmd;
       ss >> cmd;
       if( cmd == "help" ) 
       {
         fc::cout<<"mine\n";
         fc::cout<<"lookup  BIT_ID\n";
         fc::cout<<"send BIT_ID MESSAGE\n";
         fc::cout<<"inbox - print inbox\n";
         fc::cout<<"addc - adds a contact\n";

       }
       else if( cmd == "mine" ) 
       {
          app->mine_name( idents[0].bit_id, 
                          pro->get_keychain().get_identity_key( idents[0].bit_id ).get_public_key(), 
                          idents[0].mining_effort );
       }
       else if( cmd == "lookup" )
       {
          std::string bit_id;
          ss >> bit_id;
          ilog( "record: ${rec}", ("rec",app->lookup_name(bit_id)) );
       }
       else if( cmd == "send" )
       {
          std::string bit_id;
          ss >> bit_id;

          auto opt_name_rec  = app->lookup_name(bit_id);
          ilog( "record: ${rec}", ("rec",app->lookup_name(bit_id)) );
          if( opt_name_rec )
          {
              std::string msg;
              std::getline( ss, msg );
              app->send_text_message( bitchat::private_text_message(msg), fc::ecc::public_key(opt_name_rec->pub_key), my_priv_key );
          }
       }
       else if( cmd == "log" )
       {
          std::string bit_id;
          ss >> bit_id;

       }
       else if( cmd == "inbox" )
       {
          auto inbox = pro->get_inbox();
          fc::optional<fc::ecc::public_key_data> to;
          fc::optional<fc::ecc::public_key_data> from;
          auto msgs = inbox->fetch_headers( bitchat::private_message_type::text_msg, fc::time_point_sec(), fc::time_point::now(), to, from );
          ilog( "inbox: ${inbox}", ("inbox", fc::json::to_pretty_string(msgs) ) );
       }
       else if( cmd == "addc" )
       {
          std::string label;
          std::string bit_id;
          ss >> label;
          ss >> bit_id;

          fc::optional<bts::addressbook::contact>   con =   pro->get_addressbook()->get_contact_by_bitname( bit_id );
          if( con )
          {
              con->first_name = label;
              pro->get_addressbook()->store_contact( *con );
          }
          else
          {
              bts::addressbook::contact con;
              con.bitname_id = bit_id;
              con.first_name = label;
              pro->get_addressbook()->store_contact( con );
          }
       }
       else if( cmd == "contacts" )
       {
          auto names = pro->get_addressbook()->get_known_bitnames();
          fc::cout << fc::json::to_pretty_string(names) <<"\n";
       }

       fc::cout<<"\n$] ";
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
