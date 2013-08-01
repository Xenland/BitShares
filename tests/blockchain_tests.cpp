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
#include <bts/blockchain/blockchain_db.hpp>
#include <bts/config.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>

using namespace bts;
using bts::blockchain::asset;
using bts::blockchain::price;
using namespace bts::blockchain;

BOOST_AUTO_TEST_CASE( mining_reward_rate )
{
  uint64_t supply = 0;
  for( uint32_t i = 0; i < BLOCKS_WITH_REWARD; ++i )
  {
     supply += bts::blockchain::calculate_mining_reward(i);
     if( i == 0 ) supply /= 2; // dividends from gensis block do not count toward supply
  }
  ilog( "INITIAL_REWARD ${i}", ("i",INITIAL_REWARD) );
  ilog( "BLOCKS_WITH_REWARD ${i}", ("i",BLOCKS_WITH_REWARD) );
  ilog( "REWARD_DELTA_PER_BLOCK ${i}", ("i",REWARD_DELTA_PER_BLOCK) );
  ilog( "LAST_BLOCK_REWARD ${i}", ("i",calculate_mining_reward(BLOCKS_WITH_REWARD)) );
  ilog( "LAST_BLOCK_REWARD+1 ${i}", ("i",calculate_mining_reward(BLOCKS_WITH_REWARD+1)) );
  ilog( "end supply: ${e}  target supply ${t}", ("e",supply)("t",MAX_BITSHARE_SUPPLY) );
  ilog( "error: ${e}       error per block: ${epp}", ("e", (supply-MAX_BITSHARE_SUPPLY) )("epp", (supply-MAX_BITSHARE_SUPPLY)/BLOCKS_WITH_REWARD ) );
  BOOST_REQUIRE( supply == MAX_BITSHARE_SUPPLY );
  BOOST_REQUIRE( 0 == calculate_mining_reward(BLOCKS_WITH_REWARD+1));
  BOOST_REQUIRE( 0 == calculate_mining_reward(BLOCKS_WITH_REWARD));
}

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

/**
 *  Test the process of validating the block chain given
 *  a known initial condition and fixed transactions. 
 */
BOOST_AUTO_TEST_CASE( blockchain_build )
{
  try 
  {
     fc::ecc::private_key k1 = fc::ecc::private_key::generate_from_seed( fc::sha256::hash( "block1", 6 ) );
     bts::address a1 = k1.get_public_key();
     fc::ecc::private_key k2 = fc::ecc::private_key::generate_from_seed( fc::sha256::hash( "block2", 6 ) );
     bts::address a2 = k2.get_public_key();
     fc::ecc::private_key k3 = fc::ecc::private_key::generate_from_seed( fc::sha256::hash( "block3", 6 ) );
     bts::address a3 = k3.get_public_key();
     fc::ecc::private_key k4 = fc::ecc::private_key::generate_from_seed( fc::sha256::hash( "addre4", 6 ) );
     bts::address a4 = k4.get_public_key();
     
     fc::temp_directory temp_dir;
     bts::blockchain::blockchain_db chain;
     chain.open( temp_dir.path() / "chain" );
    
     auto genesis = create_genesis_block();
     ilog( "genesis block: \n${s}", ("s", fc::json::to_pretty_string(genesis) ) );
     chain.push_block( genesis );

     // build next block... 
     auto block1 = chain.generate_next_block( a1, std::vector<signed_transaction>() );
     ilog( "next block: \n${s}", ("s", fc::json::to_pretty_string(block1) ) );
     chain.push_block( block1 );

     auto block2 = chain.generate_next_block( a2, std::vector<signed_transaction>() );
     ilog( "next block: \n${s}", ("s", fc::json::to_pretty_string(block2) ) );
     chain.push_block( block2 );

     std::vector<signed_transaction> new_trx;
     new_trx.resize(1);

     new_trx[0].inputs.push_back( 
        trx_input( output_reference( block1.trxs[0].id(), 0 ) ) );

     new_trx[0].outputs.push_back( 
        trx_output( claim_by_signature_output( address( a4 ) ), 1000000, asset::bts ) );

     new_trx[0].sign( k1 );

     auto block3 = chain.generate_next_block( a3, new_trx );
     ilog( "next block: \n${s}", ("s", fc::json::to_pretty_string(block3) ) );
     chain.push_block( block3 );
  } 
  catch ( const fc::exception& e )
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
