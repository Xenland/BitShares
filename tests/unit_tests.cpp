#define BOOST_TEST_MODULE BitSharesTest
#include  <boost/test/unit_test.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/io/json.hpp>
#include "../src/blockchain.hpp"
#include "../src/chain_state.hpp"

/**
 *  This test will validate that a transaction signed with a private
 *  key will result in the address showing up in get signed addresses
 *  used for validation.
 */
BOOST_AUTO_TEST_CASE( transaction_signing_and_verification )
{
    fc::ecc::private_key dst = fc::ecc::private_key::generate();

    signed_transaction trx;
    
    trx.outputs.resize(1);
    trx.outputs[0].set_claim_function( claim_with_address( dst.get_public_key() ) );

    trx.sign_with_key( dst );

    auto saddr = trx.get_signed_addresses();
    BOOST_REQUIRE( saddr[0] == address(dst.get_public_key()) );
}



BOOST_AUTO_TEST_CASE( chain_state_commit_undo )
{
   chain_state cs;


}
