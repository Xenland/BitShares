#pragma once
#include "api.hpp"
#include <fc/thread/thread.hpp>

class account;


/**
 *  The miner will monitor the block chain for changes and
 *  attempt to solve new blocks as they come in.  When a new
 *  block is solved it will be 'added' to the chain.
 */
class miner
{
    public:
       miner( block_chain& bc, account& a );
       ~miner();

       void start( float effort = 1);
       void stop();

    private:
       fc::thread     _mining_thread;
       block_chain&   _block_chain;
       account&       _mining_account;
       float          _effort;
};
