#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>
#include <fc/io/raw.hpp>
#include <bts/mini_pow.hpp>

namespace bts { namespace bitname {

    /**
     *  Every name registration requires its own proof of work, this proof of work
     *  is designed to take about 1 hour on a home PC.  The process of mining
     *  your own name is also performing merged mining on the block that contains
     *  the names of other individuals.  
     *
     *  The difficulty of mining your own name is about 5000x less difficult than
     *  finding the full block with the caviot being that there is a minimum
     *  mining requirement.
     */
    struct name_trx 
    {
       name_trx()
       :nonce(0){}

       /** helper method */
       mini_pow id( const mini_pow& prev )const;

       uint32_t                                 nonce;     ///< 4 increment to find proof of work
       fc::time_point_sec                       utc_sec;   ///< 8 utc seconds
       // TODO: perhaps I need a more secure hash for the mroot... 80 bits probably doesn't cut
       // it... 
       mini_pow                                 mroot;     ///< 12 mroot of all trx

       /**
        *  Options for storing the name:
        *  1) 32 byte string, 1 byte size + up to 32 bytes...33 worst case, 2 best case
        *  2) Base 64 saves 14% on length but limits character set
        *  3) 64 bit hash, the entire world could have a name with a 50% chance of a single collision
        *        - the consequence of a collision is that someone picks a different name! 
        *        - this is the fastest solution and does not generate a public list of all names
        *        - you would have to test & check to find a name
        *        - in theory this expands to more data sets / foreign characters and arbitrary length names
        *        - eliminates dynamic memory allocation
        */
       uint64_t                                 name_hash;  ///< 22 hash the name to 64 bits rather than store 32+ bits...
       fc::unsigned_int                         renewal;    ///< 30 how many times has this name been renewed.
       fc::optional<fc::ecc::public_key>        key;        ///< 31 key to assign to name, 33 bytes
       fc::optional<fc::ecc::compact_signature> cancel_sig; ///< provided instead of key if renewal is 255
    }; 


    /**
     *  Combines a name_trx with a link to the previous block, this is the full
     *  structure used to calculate / validate the hash/proof of work on a name reg trx.  
     *
     *  This is pulled out into a separate struct so that the name_trx can be combined into
     *  an effecient form to store in a block without increasing the size by 16% with redundant
     *  previous block hashes.   
     */
    struct name_header : public name_trx
    {
       name_header(){}
       name_header( const name_trx& b, const mini_pow& p )
       :name_trx(b),prev(p){}
       mini_pow id()const;
       mini_pow prev;    ///< previous block
    };

    
    /**
     *  A block is a set of name reg transactions sorted by name_hash and all of which
     *  have a minimal difficulty equal to block difficulty / number of transactions / 2.
     */
    struct name_block : public name_header
    {
        mini_pow            calc_merkle_root()const; 
        /**
         *  Assuming a hash value on a scale from 1000 to 0, the
         *  probability of finding a hash below 100 = 10% and the difficulty level would be 1000/100 = 10.
         *
         *  The probability of finding 4 below 100 would be equal to the probability of finding 1 below 25.
         *
         *  The result is that we can calculate the average difficulty of a set and divide by the number of
         *  items in that set to calculate the combined difficulty.  
         */
        mini_pow            calc_difficulty()const;

        std::vector<name_trx> registered_names;
    };

    /**
     *  This is a light-weight block for broadcasting a solved block without
     *  having to include the full name_trx which have already been broadcast
     *  across the network.
     */
    struct name_block_index 
    {
        mini_pow                  prev_block;
        std::vector<uint64_t>     registered_names;
    };


} } // namespace bts::bitname


#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::bitname::name_trx, 
    (mroot)
    (nonce)
    (utc_sec)
    (renewal)
    (name_hash)
    (key)
    (cancel_sig)
)
FC_REFLECT_DERIVED( bts::bitname::name_header, (bts::bitname::name_trx), (prev) )
FC_REFLECT_DERIVED( bts::bitname::name_block, (bts::bitname::name_header), (registered_names) )
FC_REFLECT( bts::bitname::name_block_index, (prev_block)(registered_names) )


/**
 *  Define custom serialization that conditionally includes either the public key or a 
 *  signature to cancel it.  
 */
namespace fc {  namespace raw {
    template<typename Stream>
    inline void pack( Stream& s, const bts::bitname::name_trx& t )
    {
       fc::raw::pack(s,t.nonce);
       fc::raw::pack(s,t.utc_sec);
       fc::raw::pack(s,t.mroot);
       fc::raw::pack(s,t.name_hash);
       fc::raw::pack(s,t.renewal);
       if( t.renewal == 255 && t.cancel_sig)
       {
           FC_ASSERT( !!t.cancel_sig );
           fc::raw::pack(s,*t.cancel_sig);
       }
       else
       {
           FC_ASSERT( !!t.key );
           fc::raw::pack( s, *t.key );
       }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, bts::bitname::name_trx& t )
    {
       fc::raw::unpack(s,t.nonce);
       fc::raw::unpack(s,t.utc_sec);
       fc::raw::unpack(s,t.mroot);
       fc::raw::unpack(s,t.name_hash);
       fc::raw::unpack(s,t.renewal);
       if( t.renewal != 255 )
       {
           t.key = fc::ecc::public_key();
           fc::raw::unpack( s, *t.key );
       }
       else
       {
           t.cancel_sig = fc::ecc::compact_signature();
           fc::raw::unpack( s, *t.cancel_sig );
       }
    }
} }
