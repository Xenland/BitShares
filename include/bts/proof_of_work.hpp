#pragma once
#include <fc/crypto/sha256.hpp>
#include <bts/mini_pow.hpp>

namespace bts {

    /** typedef to the same size */
    typedef mini_pow pow_hash;

    /**
     *  The purpose of this method is to generate a determinstic proof-of-work
     *  that cannot be optimized via ASIC or extreme parallelism. 
     *
     *  @param in           - initial hash
     *  @param buffer_128m  - 128 MB buffer used for scratch space.
     *  @return processed hash after doing proof of work.
     */
    mini_pow proof_of_work( const fc::sha256& in, unsigned char* buffer_128m );
    mini_pow proof_of_work( const fc::sha256& in );

}

