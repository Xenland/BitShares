#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/sha224.hpp>

namespace bts { namespace bitname {

    typedef uint64_t      name_hash_type;
    typedef fc::uint128   name_trxs_hash_type; // consider making fc::uint128
    typedef fc::sha224    name_id_type;        // full crypto-secure name id
    typedef uint64_t      short_name_id_type;  // short, bandwidth effecient, collision resitant name type

    /**
     *  Every name registration requires its own proof of work, this proof of work
     *  is designed to take about 1 hour on a home PC.  The process of mining
     *  your own name is also performing merged mining on the block that contains
     *  the names of other individuals.  
     *
     *  The block difficulty is calculated as the sum of all difficulties of
     *  included name_trx plus the difficulty of the header.  The difficulty of
     *  the header must be over 50% of the difficulty of the block.
     *
     *  The minimum difficulty for finding a name is about 1 hr of CPU time, but
     *  could be as high as block target / 10K.  
     *
     *  When a block is name_trx is found, the name earns one repute point, when
     *  a name block is found, the name the found it earns an additional repute point 
     *  for each included transaction.
     *
     *  Users mine for the following benefits:
     *    1) register a public key to their name
     *    2) gain repute_points
     *    3) renew their name / prevent expiration
     *    4) change / cancel their name.
     */
    struct name_trx 
    {
        name_trx()
        :nonce(0),age(0),trxs_hash(0),name_hash(0){}
        
        /** Helper method, given a name trx, we need the prev block hash to
         * calculate the id.  Normally this is provided with the name_header,
         * but the there is no need to construct a name_header to simply 
         * calculate the id.
         */
        name_id_type         id( const name_id_type& prev )const;
        /** short id == city64(id()) */
        short_name_id_type   short_id( const name_id_type& prev )const;
        uint64_t             difficulty( const name_id_type& prev )const;

        /** Increment to find proof of work, intentionally small to slow down mining rate
         *  to 65K/hash sec without adding new trxs (with valid proof of work) or generating
         *  a new public key, but new public keys are not available for renewals.
         *
         *  This should drive inclusion of trx in the mroot to accelerate an individuals own
         *  hashing rate.  The trxs_hash root could also be incremented assuming there was
         *  no attempt to 'solve the block'.  Why does anyone bother trying to solve the
         *  block?  Because solving the block allows you to change your public key or
         *  to cancel it.   These actions must be done at the block level so that light
         *  clients can easily verify that a key is valid and hasn't been changed.  Solving
         *  the block also gains you one renewal point for each trx included in the block and
         *  these renewal points combined with age factor into the 'credability' and 'uniqueness'
         *  of your account for web-of-trust purposes.
         */
       uint16_t                                 nonce;        

       /**
        * Timestamp of the transaction, used to calculate blockchain time.
        */
       fc::time_point_sec                       utc_sec; 

       /**
        *  First block number this public key was found in.
        */
        uint32_t                                age;      


       /**
        * Why can I get away with a 64 bit hash without fear of collisions?  Because each
        * of the transactions themselves has a proof of work associated with it, an attacker
        * would have to find a collision that simultainously satisfies the proof of work
        * requirements of the trx.  Each attempt at a collision would require about 5 minutes
        * of CPU time or more (120K attempts/year per CPU) by which time the block would have expired.
        *
        * What could be gained by finding a collision? You could perhaps fool a light-client
        * and convince them you own a name you do not or change the reputation.  These attacks
        * would not work against full clients.  By using 64 bits I reduce the storage and
        * bandwidth requirements significantly considering the entire name_trx struct is less
        * than 60 bytes.
        *
        * Note: with all of the power of the bitcoin network, a 64 bit collision can only
        * be found a couple of times per year and the cost per hash is billions of times
        * less.
        */
       name_trxs_hash_type                       trxs_hash;        

       /**
        *  64 bit hash of name, the entire world could have a name with a 50% chance of a single collision
        *    - the consequence of a collision is that someone picks a different name! 
        *    - this is the fastest solution and does not generate a public list of all names
        *    - you would have to test & check to find a name
        *    - in theory this expands to more data sets / foreign characters and arbitrary length names
        *    - eliminates dynamic memory allocation
        *
        *    The first 1000 name_hash slots are reserved for protocol updates.  0 is reserved
        *    for gensis block, the rest can be used to indicate additional data that may be
        *    serialized as part of the name_trx for future growth.
        */
       name_hash_type                           name_hash;         

       /**
        *  If the key is changing or being canceled, renewal_points must be 0 
        *  and an optional signature can then be provied.
        */
       fc::unsigned_int                         repute_points;  
       /**
        *  Public key, must be the same as the prior public key unless this
        *  is a new registration or solves the block.
        */
       fc::ecc::public_key_data                 key;           

       /**
        * Only valid when repute_points is 0 and indicates that a name
        * has changed owners or been canceled.  A name is canceled (in the event
        * of a conflict) if an update with a null key is provided.  A name
        * can be canceled within 24 hours of a key change using the prior key.
        */
       fc::optional<fc::ecc::compact_signature> change_sig;   
    };  // name_trx


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
       name_header( const name_trx& b, const name_id_type& p )
       :name_trx(b),prev(p){}

       uint64_t   difficulty()const;
       name_id_type       id()const;
       short_name_id_type short_id()const;
       name_id_type       prev;    ///< previous block
    };

    
    /**
     *  A block is a set of name reg transactions sorted by name_hash and all of which
     *  have a minimal difficulty equal to block difficulty / number of transactions / 2.
     */
    struct name_block : public name_header
    {
        name_block(){}

        name_block( const name_header& h )
        :name_header(h){}

        name_trxs_hash_type  calc_trxs_hash()const; 

        /**
         *   Assuming all registered_names meet the current 
         *   target difficulty / 10K and are all greater than
         *   the minimum difficulty, simply multiple the
         *   number of registered names * name target difficulty
         *   and then add the difficulty of this header.
         */
        uint64_t            block_difficulty()const;

        /**
         *  Each name requires target_difficulty / 10K or Min
         *  POW.
         *  
         *  TODO: verify name_trx are all unique!
         */
        std::vector<name_trx> registered_names;
    };

    /**
     *  This is a light-weight block for broadcasting a solved block without
     *  having to include the full name_trx which have already been broadcast
     *  across the network.
     */
    struct name_block_index 
    {
        name_block_index(){}
        name_block_index( const name_block& block )
        :header(block)
        {
          for( auto itr = block.registered_names.begin();
                    itr != block.registered_names.end();
                    ++itr )
          {
            registered_names.push_back( itr->name_hash );
          }
        }
        name_header                  header;
        std::vector<name_hash_type>  registered_names;
    };


    name_block create_genesis_block();
    uint64_t    min_name_difficulty();
} } // namespace bts::bitname


#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::bitname::name_trx, 
    (nonce)
    (age)
    (utc_sec)
    (trxs_hash)
    (name_hash)
    (repute_points)
    (key)
    (change_sig)
)
FC_REFLECT_DERIVED( bts::bitname::name_header, (bts::bitname::name_trx), (prev) )
FC_REFLECT_DERIVED( bts::bitname::name_block, (bts::bitname::name_header), (registered_names) )
FC_REFLECT( bts::bitname::name_block_index, (header)(registered_names) )


/**
 *  Define custom serialization that conditionally includes either the public key or a 
 *  signature to cancel it.  
 */
namespace fc {  namespace raw {
    template<typename Stream>
    inline void pack( Stream& s, const bts::bitname::name_trx& t )
    {
       fc::raw::pack(s,t.nonce);
       fc::raw::pack(s,t.age);
       fc::raw::pack(s,t.utc_sec);
       fc::raw::pack(s,t.trxs_hash);
       fc::raw::pack(s,t.name_hash);
       fc::raw::pack(s,t.repute_points);
       fc::raw::pack(s,t.key);
       if( t.repute_points.value == 0 )
       {
           FC_ASSERT( !!t.change_sig );
           fc::raw::pack(s,*t.change_sig);
       }
    }

    template<typename Stream, typename T>
    inline void unpack( Stream& s, bts::bitname::name_trx& t )
    {
       fc::raw::unpack(s,t.nonce);
       fc::raw::unpack(s,t.age);
       fc::raw::unpack(s,t.utc_sec);
       fc::raw::unpack(s,t.trxs_hash);
       fc::raw::unpack(s,t.name_hash);
       fc::raw::unpack(s,t.repute_points);
       fc::raw::unpack(s,t.key);

       if( t.repute_points.value == 0 )
       {
           t.change_sig = fc::ecc::compact_signature();
           fc::raw::unpack( s, *t.change_sig );
       }
    }
} }
