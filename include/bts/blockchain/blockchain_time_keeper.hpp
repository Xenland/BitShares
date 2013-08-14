#pragma once
#include <fc/time.hpp>
#include <fc/crypto/bigint.hpp>
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
        time_keeper( fc::time_point origin_time, 
                     fc::microseconds block_interval, 
                     uint32_t window = 1024 ); // no reason for window to be larger than allowed time variance
        ~time_keeper();

        void push( uint32_t block_num, fc::time_point block_time, fc::bigint block_proof_of_work );

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
        fc::bigint next_difficulty()const;             

        /**
         * Return the current difficulty level as the
         * average of all blocks in the window.
         */
        fc::bigint current_difficulty()const;

     private:
        std::unique_ptr<detail::time_keeper_impl> my;
  };

} } // bts::blockchain
