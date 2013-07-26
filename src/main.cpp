//#include "api.hpp"
#include "miner.hpp"
#include "account.hpp"
#include "wallet.hpp"
#include <fc/io/stdio.hpp>
#include <fc/io/json.hpp>
#include <sstream>
#include <iostream>


struct bitshare_config
{
   fc::path    data_dir;
   float       mine_effort;

};
FC_REFLECT( bitshare_config, 
            (data_dir)
            (mine_effort) 
          ) 

/**
 *  Prints out the account with balances and available outputs,
 *  dividends, etc.
 */
void print_account( account& a, block_chain& bc )
{
     auto  addrs    = a.get_addresses();
     std::vector<output_cache> outs;
     std::cout<<" Account: '"<<a.name()<<"'\n";
     std::cout<<"----------------------------------------------------------------\n";
     std::cout<<" Addresses: \n";
     for( auto itr = addrs.begin(); itr != addrs.end(); ++itr )
     {
        std::cout<<"   "<<std::string(*itr)<<"\n"; // TODO: print whether or not we have the private key
        auto ve = bc.get_outputs_for_address( *itr );
        outs.insert( outs.end(), ve.begin(), ve.end() );
     }

     std::cout<<" Outputs: \n";
     for( auto itr = outs.begin(); itr != outs.end(); ++itr )
     {
        std::cout<<"     "<< bc.pretty_print_output( *itr ) <<"\n";
     }
}

int main( int argc, char** argv )
{
   try
   {
      if( argc < 2 )
      {
         fc::cerr<<"Usage: "<<argv[0]<<" config\n";
         return -1;
      }
      bitshare_config cfg;
      try {
        cfg = fc::json::from_file( fc::path( argv[1] ) ).as<bitshare_config>();
      } 
      catch( fc::eof_exception& e )
      {
        fc::cerr<<"Error loading configuration\n"<<e.to_detail_string()<<"\n";
        return -1;
      }
      
      // load the block-chain
      block_chain bc;
      bc.load( cfg.data_dir / "chain" );

      // load my account
      account default_acnt;
      default_acnt.load( cfg.data_dir / "accounts" / "default", account::create );

      //bc.output_added.connect( [&]( const output_cache& o ) { default_acnt.add_output(o); } );
      //bc.output_removed.connect( [&]( const output_cache& o ) { default_acnt.remove_output(o); } );

      // start the miner
      miner bc_miner( bc, default_acnt );
      bc_miner.start( cfg.mine_effort );

      // start the network
      // node  bsnode(bc);

      // start the CLI
      fc::string line;
      fc::getline( fc::cin, line );
      while( line != "quit" )
      {
          std::string sline(line);
          std::stringstream ss(sline);
          std::string cmd;

          ss >> cmd;

          if( cmd == "help" )
          {
             fc::cout<<"Commands:\n";
             fc::cout<<"add_block\n";
             fc::cout<<"mine effort\n";
             fc::cout<<"print_chain\n";
             fc::cout<<"print_account\n";
             fc::cout.flush();
          }
          else if( cmd == "add_block" )
          {
             auto coinbase_addr  = default_acnt.get_new_address();
             block  new_block    = bc.generate_next_block( coinbase_addr );

             fc::cout<<"Creating new block:\n"<< fc::json::to_pretty_string(new_block) <<"\n";

             bc.add_block( new_block );

            // adds the next block without mining...
          }
          else if( cmd == "mine" )
          {
              float effort = 0;
              ss >> effort;
              bc_miner.start( effort );
          }
          else if( cmd == "print_chain" )
          {
              bc.pretty_print_chain();
          }
          else if( cmd == "print_account" )
          {
              print_account(default_acnt, bc);
          }
          fc::getline( fc::cin, line );
      }
   } 
   catch ( fc::eof_exception& e )
   {
        fc::cerr<<"eof\n";
        // expected end of cin..
        return 0;
   }
   catch ( fc::exception& e )
   {
      fc::cerr<<e.to_detail_string()<<"\n";
      return -1;
   }
   return 0;
}
