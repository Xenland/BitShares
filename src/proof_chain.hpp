#pragma once

/**
 *  A chain of hashes that ends with the hash of a
 *  block header along with the nonce that when
 *  combined with the head of the chain results in
 *  a hash that can satisify the proof of work.
 */
class proof
{
    public:
    uint64_t                  nonce;
    std::vector<fc::sha224>   merkchain; 
};


