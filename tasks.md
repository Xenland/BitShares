If you would like to work on these tasks please contact a project lead and they
will give you dibs on them provided you commit to a completion date.  Fork the code,
and generate a pull request when complete.  After a code review, your patch will
be imported into the code code base and your funds will be paid.

0.5 BTC  -  fc hashing refactor:   
-------------------------------
   replace fc::sha224  fc::sha256 fc::sha512 fc::sha1 with a single generic template class of the proper byte bitsiz
        and then typedef define fc::uint160, 224, 256, 512 to that template type.
   
   move fc::shaX::encoder   to fc::shaX_encoder  and update to produce uintX
   move fc::shaX::hash(...) to fc::shaX_hash(...)
   update all of BitShares to compile with these changes.

1  BTC - generate an input via socket connection that can cause our client to crash.
--------------------------------
   not yet open for dibs, but will be opened after we complete our initial security audit.
   must provide unit test that demonstrates the crash in reproduceable manner.

2  BTC - generate full coverage tests for bts::bitname::fork_db
--------------------------------
    not yet open for dibs, but will be once the full funtionality is defined.  Let me
    know if you are interested.

    must generate chains at least 2 years deep at 1 header every 5 minutes
    must handle populating the blocks in any order and generate the same result.
    must benchmark get_forks(), set_valid()
    must verify that the contents of unknown is always accurate
    must be in the form of a boost unit test.
    must check setting and clearing the valid state on random nodes
    must exersize every method and every branch in the code (verify this somehow?)


1 BTC - update stcp_socket to use fc::aes_encoder 
--------------------------------
    open for bids now.
    must verify that communication via bts::network::connection remains functional.

.25 BTC - implement from_variant in src/blockchain/transaction.cpp
   open for bids now
   void from_variant( const variant& var,  bts::blockchain::trx_output& vo )
   - must provide boost unit tests for all variations.
