#pragma once
#include <fc/crypto/sha256.hpp>
#include <fc/uint128.hpp>

namespace bts {

    /** typedef to the same size */
    typedef fc::uint128 pow_hash;

    /**
     *  The purpose of this method is to generate a determinstic proof-of-work
     *  that cannot be optimized via ASIC or extreme parallelism. 
     *
     *  @param in           - initial hash
     *  @param buffer_128m  - 128 MB buffer used for scratch space.
     *  @return processed hash after doing proof of work.
     */
    fc::uint128 proof_of_work( const fc::sha256& in, unsigned char* buffer_128m );
    fc::uint128 proof_of_work( const fc::sha256& in );

}

