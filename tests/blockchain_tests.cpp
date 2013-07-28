#define BOOST_TEST_MODULE BitSharesTest
#include <boost/test/unit_test.hpp>
#include <bts/wallet.hpp>
#include <bts/address.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <bts/mini_pow.hpp>
#include <bts/dds/mmap_array.hpp>
#include <bts/blockchain/asset.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>

using namespace bts;
using bts::blockchain::asset;
using bts::blockchain::price;

BOOST_AUTO_TEST_CASE( fixed_math )
{
  try{
   asset usd( 30000, asset::usd );
   asset bts( 5000, asset::bts );
   auto  usd_bts = usd / bts;
   auto  bts_usd = bts / usd;
   
   ilog( "usd: ${u}  bts: ${b}", ("u",usd)("b",bts) );
   ilog( "usd_bts = ${ub}   bts_usd = ${bu}", ("ub", usd_bts)("bu",bts_usd) );
    
   asset usd56( 5, asset::usd );
   asset bts43( 43, asset::bts );

   auto bts43_in_usd =  bts43 * bts_usd;
   auto back_to_bts = bts43_in_usd * bts_usd;
   ilog( " ${b} * ${p}  =>  ${a}", ("b", bts43)("p",bts_usd)("a",bts43_in_usd) );
   ilog( " ${b} * ${p}  =>  ${a}", ("b", bts43_in_usd)("p",bts_usd)("a",back_to_bts) );
   BOOST_CHECK( back_to_bts.amount.high_bits() == bts43.amount.high_bits() );
   {
   asset usd( 5, asset::usd );
   asset bts( 3, asset::bts );
   auto  usd_bts = usd / bts;
   auto  bts_usd = bts / usd;
   
   ilog( "usd: ${u}  bts: ${b}", ("u",usd)("b",bts) );
   ilog( "usd_bts = ${ub}   bts_usd = ${bu}", ("ub", usd_bts)("bu",bts_usd) );
   }

  } catch ( const fc::exception& e )
  {
    elog( "${e}", ("e",e.to_detail_string()) );
    throw;
  }
}

BOOST_AUTO_TEST_CASE( bts_address )
{
  try 
  {
     auto priv = fc::ecc::private_key::generate();
     bts::address addr =  priv.get_public_key();
     std::string addrstr = addr;
     bts::address addr2 = addrstr;
     bts::address addr3 =  priv.get_public_key();
     
     BOOST_REQUIRE( addr == addr2 );
     BOOST_REQUIRE( addr3 == addr2 );
     BOOST_REQUIRE_THROW( bts::address("invalid"), fc::exception);
     BOOST_REQUIRE_THROW( bts::address("a23"), fc::exception);
     ilog( "address ${adr}", ("adr", addrstr ) );
     ilog( "address ${adr}", ("adr", addr ) );
  } catch( fc::exception& e )
  {
    elog( "${e}", ("e",e.to_detail_string()) );
    throw;
  }
}
