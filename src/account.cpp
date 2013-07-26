#include "account.hpp"
#include "wallet.hpp"
#include "meta.hpp"
#include <fc/io/json.hpp>
#include <fc/io/fstream.hpp>
#include <fc/filesystem.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/reflect/variant.hpp>
#include <unordered_set>

/**
 *  Caches all transactions that contain an input
 *  or output that references an address managed
 *  by this wallet. 
 */
struct account_file
{
   fc::string                      name;
   std::vector<address>            free_addresses;
   std::unordered_set<address>     used_addresses;
};

FC_REFLECT( account_file, (name)(free_addresses)(used_addresses) )

namespace detail 
{

   class account_impl
   {
      public:
         fc::path                                          _account_dir;
         fc::path                                          _wallet_file;
         fc::path                                          _account_file;
         wallet                                            _wallet;
         account_file                                      _account;
   };

}


account::account()
:my( new detail::account_impl() )
{}


account::~account(){}


void account::load( const fc::path& account_dir, account::open_mode m )
{
    my->_account_file = account_dir / "account.bsaj";
    my->_wallet_file  = account_dir / "wallet.dat";
    if( !fc::exists( account_dir )  ) 
    {
       if( m != create )
       {
        FC_THROW_EXCEPTION( file_not_found_exception, "Account directory ${dir} does not exist", ("dir",account_dir) );
       }
       fc::create_directories( account_dir );
    }
    if( fc::exists( my->_account_file ) ) 
       my->_account = fc::json::from_file<account_file>( my->_account_file );
}

void account::set_name( const std::string& name )
{
    my->_account.name = name;
}

std::string account::name()const { return my->_account.name; }


void account::save()
{
    auto a = fc::json::to_pretty_string( my->_account );
    {
      fc::ofstream out( my->_account_file.generic_string() + ".tmp" );
      out.write( a.c_str(), a.size() );
    }
    if( fc::exists( my->_account_file ) )
    {
       fc::rename( my->_account_file, my->_account_file.generic_string() + ".back" );
       fc::rename( my->_account_file.generic_string()+".tmp", my->_account_file );
       fc::remove( my->_account_file.generic_string() + ".back" );
    }
    else
    {
       fc::rename( my->_account_file.generic_string()+".tmp", my->_account_file );
    }
}


wallet& account::get_wallet() 
{ 
    return my->_wallet; 
}

wallet& account::load_wallet(const std::string& pass ) 
{ 
    if( fc::exists( my->_wallet_file ) )
    {
        my->_wallet.load( my->_wallet_file, pass );
    }
    return my->_wallet; 
}
void account::save_wallet( const std::string& pass )
{
     my->_wallet.save( my->_wallet_file, pass );
}

void account::add_address( const address& a )
{
    my->_account.used_addresses.insert(a);
}

address account::get_new_address()
{
   if( !my->_account.free_addresses.size() )
   {
      if( !get_wallet().is_locked() )
      {
         my->_account.free_addresses = get_wallet().reserve( 100 );
      }
      else
      {
         FC_THROW_EXCEPTION( exception, "no addresses available and the wallet is locked" );
      }
   }
   auto naddr = my->_account.free_addresses.back();
   my->_account.used_addresses.insert( naddr );
   my->_account.free_addresses.pop_back();
   return naddr;
}

const std::unordered_set<address>& account::get_addresses()const
{
    return my->_account.used_addresses;
}

bool account::contains( const address& addr ) 
{
  return my->_account.used_addresses.find(addr) != my->_account.used_addresses.end();
}

