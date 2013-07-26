#include "miner.hpp"


miner::miner( block_chain& bc, account& a )
:_block_chain(bc),_mining_account(a)
{
}

miner::~miner()
{
}

void miner::start( float effort )
{
}

void miner::stop()
{
}

