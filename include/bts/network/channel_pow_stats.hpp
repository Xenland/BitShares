#pragma once
#include <bts/mini_pow.hpp>

namespace bts { namespace network {
  
  /**
   *  Tracks the bandwidth used by a specific channel and calculates
   *  the average proof-of-work per message to get an idea of what
   *  is required to broadcast on the network.  
   *
   *  If the bandwidth is above the target bandwidth over the last
   *  5 minutes then the required proof-of-work for rebroadcast is
   *  5% above average, otherwise it is 5% below average.
   */
  class channel_pow_stats
  {
    public:
       channel_pow_stats();

       /**
        *  Updates the weighted-average bits per microsecond handled by
        *  a particular channel using the system clock to measure elapsed
        *  time between calls.
        *
        *  @param bytes_received the bytes received since the last call
        *  @param msg_pow  the proof of work associated with these bytes.
        *
        *  @pre msg_pow < target_pow
        *
        *  @return true if these bytes can be received / processed at the given
        *          proof of work level.
        */
       bool update_bpus_avg( uint64_t bytes_recv, const mini_pow& msg_pow );

       mini_pow        target_pow;
       mini_pow        average_pow;
       uint64_t        target_bits_per_usec;
       uint64_t        avg_bits_per_usec;
       fc::time_point  last_recv;
  };
  
} }

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::network::channel_pow_stats, 
    (target_pow)
    (average_pow)
    (target_bits_per_usec)
    (avg_bits_per_usec)
    (last_recv) 
    )
  


