#pragma once
#include <bts/bitname/bitname_block.hpp>

namespace bts { namespace bitname {

    /**
     *  Defines the call back methods when a name is found.
     */
    class name_miner_delegate 
    {
       public:
          virtual ~name_miner_delegate(){}

          /**
           *  Called with a block that satisifies the current block
           *  target difficulty.
           */
          virtual void found_name_block( const name_block& t ){};

          /**
           *  Called when a nonce good enough for broadcasting a
           *  trx is found.  Mining will continue in an attempt to
           *  solve the block until the miner is told otherwise.
           */
          virtual void found_name_trx( const name_trx& t ){};
    };

    namespace detail { class name_miner_impl; }

    /**
     *  @brief Searches for a nonce that will satisify the target difficulty.
     *
     *  The name miner should be given a target difficulty and a level of effort
     *  and will search the nonce space until the difficulty is met.  Part of
     *  mining your name is also merged-mining for the entire block and so including
     *  other names in the process can earn you additional reputation points.
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
          void set_block_target( uint64_t difficulty );
          void set_name_trx( const name_trx& t );

          /**
           *  This will clear all all names from the trx queue.
           */
          void set_prev( const fc::sha224& prev_block );

          /**
           *  If another transaction with the same name is already
           *  in the queue, the lowest hash will win. 
           *
           *  @note miner does not perform any validation aside from checking
           *  for duplicates in the same block.  Validation should  be 
           *  performed prior to this call.
           */
          void add_name_trx( const name_trx& );

          void start( float effort = 1 );
          void stop();

       private:
          std::unique_ptr<detail::name_miner_impl> my;
    };

} }  // namespace bts
