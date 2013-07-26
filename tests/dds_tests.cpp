#define BOOST_TEST_MODULE BitSharesTest
#include  <boost/test/unit_test.hpp>
#include <bts/wallet.hpp>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <fc/reflect/variant.hpp>
#include <bts/bitmessage.hpp>
#include <bts/bitchat_message.hpp>
#include <bts/mini_pow.hpp>
#include <bts/dds/mmap_array.hpp>
#include <fc/io/json.hpp>
#include <fc/io/raw.hpp>

using namespace bts;
BOOST_AUTO_TEST_CASE( mini_pow_test )
{
  std::string hello_world( "hello world");
  auto p = bts::mini_pow_hash( hello_world.c_str(), hello_world.size() );
  ilog("p: ${p}", ("p",p));
  auto p2 = bts::mini_pow_hash( hello_world.c_str(), hello_world.size() );
  ilog("p2: ${p}", ("p",p2));
  BOOST_CHECK( p == p2 );
  ilog("");

  uint32_t tmp[8];
  memset( (char*)tmp, 0, sizeof(tmp) );
  ilog("");

  uint8_t* first = (uint8_t*)&p;
  uint8_t min = 255;
  while( *first > 225 )
  {
      tmp[0]++;
      p = bts::mini_pow_hash( (char*)tmp, sizeof(tmp) );

      if( *first < min ) 
      {
         ilog( "found ${h}", ("h",p) );
         min = *first;
      }
  }

}

BOOST_AUTO_TEST_CASE( mmap_array_test )
{
  try {
    fc::temp_directory temp_dir;
    mmap_array<fc::sha224> hasha;
    hasha.open( temp_dir.path() / "test.array" );

    BOOST_CHECK( hasha.size() == 0 );
    ilog("");
    hasha.resize( 1024 );
    ilog("");
    BOOST_CHECK( hasha.size() == 1024 );
    ilog("");
    hasha.at(4) = fc::sha224::hash( "hello", 5 );
    ilog("");
    hasha.close();
    hasha.open( temp_dir.path() / "test.array" );
    BOOST_CHECK( hasha.size() == 1024 );
    BOOST_CHECK( hasha.at(4) == fc::sha224::hash( "hello", 5 ) );
  } catch ( fc::exception& e )
  {
     elog( "${e}", ("e",e.to_detail_string() ) );
     throw;
  }
}



BOOST_AUTO_TEST_CASE( wallet_test )
{
/* TODO: this test is slow...
   bts::wallet w;
   w.set_seed( fc::sha256::hash( "helloworld", 10 ) );

   bts::wallet w2;
   w2.set_master_public_key( w.get_master_public_key() );

   for( uint32_t i = 0; i < 10; ++i )
   {
      BOOST_CHECK(  bts::address(w.get_public_key(i))                   == w2.get_public_key(i) );
      BOOST_CHECK(  bts::address(w.get_private_key(i).get_public_key()) == w2.get_public_key(i) );
   }
   */
}

/*
BOOST_AUTO_TEST_CASE( bitmessage_test )
{
  try {
    fc::ecc::private_key from = fc::ecc::private_key::generate();
    fc::ecc::private_key toA  = fc::ecc::private_key::generate();
    fc::ecc::private_key toB  = fc::ecc::private_key::generate();

    bts::bitmessage m;
    m.to( toA.get_public_key() );
    m.subject( "hello world" );
    m.body( "my body is cool" );
    m.sign( from );

    m.encrypt();

    ilog( "from: ${f}", ("f",from.get_public_key().serialize()) );
    ilog( "encrypted message: ${m}", ("m",fc::json::to_pretty_string( m )) );
    ilog( "encrypted content: ${c}", ("c",fc::json::to_pretty_string( m.get_content() )) );
    
    auto packed = fc::raw::pack(m);
    auto unpacked = fc::raw::unpack<bts::bitmessage>(packed);

    BOOST_CHECK( unpacked.decrypt( toA ) );
    ilog( "decrypted content: ${c}", ("c",fc::json::to_pretty_string( unpacked.get_content() )) );
    BOOST_CHECK( unpacked.get_content().from );
    ilog( "signed by: ${f}", ("f",unpacked.get_content().from->serialize()) );
    BOOST_CHECK( unpacked.get_content().from->serialize() == from.get_public_key().serialize() );
    ilog( "message size: ${s}", ("s", packed.size() ) );

    unpacked = fc::raw::unpack<bts::bitmessage>(packed);
    BOOST_CHECK( !unpacked.decrypt( toB ) );
  } catch ( fc::exception& e )
  {
     elog( "${e}", ("e",e.to_detail_string() ) );
     throw;
  }
}

BOOST_AUTO_TEST_CASE( bitchat_message_test )
{
  try {
    fc::ecc::private_key from = fc::ecc::private_key::generate();
    fc::ecc::private_key toA  = fc::ecc::private_key::generate();
    fc::ecc::private_key toB  = fc::ecc::private_key::generate();

    std::vector<char> rand_body(12);
    memcpy( rand_body.data(), "hello world", 12 );

    bts::bitchat_message m;
    m.body( rand_body );
    m.sign( from );
    m.encrypt( toA.get_public_key() );

    ilog( "from: ${f}", ("f",from.get_public_key().serialize()) );
    ilog( "encrypted message: ${m}", ("m",fc::json::to_pretty_string( m )) );
    ilog( "encrypted content: ${c}", ("c",fc::json::to_pretty_string( m.get_content() )) );
    
    auto packed = fc::raw::pack(m);
    auto unpacked = fc::raw::unpack<bts::bitchat_message>(packed);

    BOOST_CHECK( unpacked.decrypt( toA ) );
    ilog( "decrypted content: ${c}", ("c",fc::json::to_pretty_string( unpacked.get_content() )) );
    BOOST_CHECK( unpacked.get_content().from );
    ilog( "signed by: ${f}", ("f",unpacked.get_content().from->serialize()) );
    BOOST_CHECK( unpacked.get_content().from->serialize() == from.get_public_key().serialize() );
    ilog( "message size: ${s}", ("s", packed.size() ) );

    unpacked = fc::raw::unpack<bts::bitchat_message>(packed);
    BOOST_CHECK( !unpacked.decrypt( toB ) );
  } 
  catch ( fc::exception& e )
  {
     elog( "${e}", ("e",e.to_detail_string() ) );
     throw;
  }
}
*/
