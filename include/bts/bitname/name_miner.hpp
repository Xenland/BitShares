#pragma once
#include <bts/bitname/name_block.hpp>

namespace bts { namespace bitname {

    class name_miner_delegate 
    {
       public:
          virtual ~name_miner_delegate(){}
          virtual void found_name_block( const name_block& t ){};
         
    };

    namespace detail { class name_miner_impl; }

    /**
     *  @brief Searches for a nonce that will satisify the target difficulty.
     *
     *  The name miner should be given a target difficulty and a level of effort
     *  and will search the nonce space until the difficulty is met.  Part of
     *  mining your name is also merged-mining for the entire block and so including
     *
     */
    class name_miner
    {
       public:
          name_miner();
          ~name_miner();

          void set_delegate( name_miner_delegate* d );

          /** Sets the hash value required for finding a block.  The
           *  difficulty of finding a name is 1/5000 * the block difficulty 
           *  with a minimum difficulty of about 1 hour per CPU.
           */
          void set_block_target( const mini_pow& p );

          void set_name( const std::string& name, const fc::ecc::public_key& k );

          void start( float effort = 1 );
          void stop();

          /**
           *  This will clear all all names from the merkle tree that
           *  don't share the same prev pow.
           */
          void set_prev( const mini_pow& p );

          /**
           *  If another transaction with the same name is already
           *  in the queue, the lowest hash will win.  Recalculates the
           *  merkle root.
           */
          void add_name_trx( const name_trx& );
       private:
          std::unique_ptr<detail::name_miner_impl> my;
    };

} }  // namespace bts
