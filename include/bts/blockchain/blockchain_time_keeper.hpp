#pragma once
#include <fc/time.hpp>
#include <memory>

namespace bts { namespace blockchain {

  namespace detail { class time_keeper_impl; }
  /**
   *  This is a utility class designed to coordinate
   *  blockchain timestamps and difficulty using
   *  a sliding window.
   *  
   *  Given a set of timestamps and corresponding
   *  proof of work hashes, this class will calculate
   *  the proof of work required for the next
   *  block as well as the minimum allowable
   *  time for the next block.
   *
   *  The difficulty will be adjusted so that the
   *  expected time of finding the next block will
   *  cause the time series to re-sync with the
   *  desired time interval.
   */
  class time_keeper
  {
     public:
        time_keeper();
        ~time_keeper();
        void configure( fc::time_point origin, fc::microseconds interval, uint32_t window = 4096 );

        /**
         *   Used to populate the initial state of the time keeper 
         */
        void push_init( uint32_t block_num, fc::time_point block_time, uint64_t block_difficulty );

        /**
         *  After pushing all init state, calculate the initial stats and then start enforcing
         *  requirements on new blocks being pushed.
         */
        void init_stats();

        /**
         *  @param  difficulty = a measure of how hard it is to find a block, this number
         *          increases to make it take longer to find a block, and decreases to
         *          cause blocks to be found faster.  It is recommend to use fixed point
         *          with at least 6 decimial places of precision, but the time_keeper doesn't
         *          particularlly care.
         *
         *  @note init_stats() must be called prior to calling this for the first time.
         */
        void push( uint32_t block_num, fc::time_point block_time, uint64_t difficulty );

        /**
         *  pop everything after block_num
         */
        void pop( uint32_t block_num ); 

        /**
         *  The next expected block number.
         */
        uint32_t next_block_num()const;

        /**
         *  Estimate the current time based only on the
         *  block history and assuming that the most recent
         *  block is included.  
         */
        fc::time_point current_time()const;

        /**
         *  The expected time for the next block based upon
         *  the current difficulty.
         */
        fc::time_point next_time()const;
        
        /**
         *  Calculate the difficulty for the next block.
         */
        uint64_t next_difficulty()const;             

        /**
         * Return the current difficulty level as the
         * average of all blocks in the window.
         */
        uint64_t current_difficulty()const;

        int64_t current_time_error()const;
        
        fc::time_point expected_time( uint32_t block_num )const;
     private:
        std::unique_ptr<detail::time_keeper_impl> my;
  };

} } // bts::blockchain
