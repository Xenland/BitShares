#include <bts/network/channel_pow_stats.hpp>
#include <fc/crypto/bigint.hpp>

namespace bts { namespace network {

channel_pow_stats::channel_pow_stats()
:target_bits_per_usec(0),avg_bits_per_usec(0)
{
  // set the initial target as high as possible
  memset( (char*)&target_pow, 0xff, sizeof(target_pow) );
}

/**
 *   TODO: unit test channel_pow_stats::update_pbs_avg
 */
bool channel_pow_stats::update_bps_avg( uint64_t bytes_recv, const pow_hash& msg_pow )
{
   // normalize the proof of work for the message size
   fc::bigint msg( &msg_pow,     sizeof(msg_pow) );
   msg *= ( (bytes_recv / 1024) + 1); // give the average work per kb

   // for the purposes of this calculation, there is no such thing as a message less than
   // 1 KB in size.
   if( bytes_recv < 1024 ) bytes_recv = 1024; 

   // first check the work, if it is valid we can work it into our
   // POW average, otherwise we will ignore it.
   if( msg_pow > target_pow ) return false;

   // how much time has elapsed since the last bytes that passed
   auto ellapsed_us = (fc::time_point::now() - last_recv).count();
   last_recv = fc::time_point::now();

   // prevent long delays between message from biasing the average too much
   // also protects against OS time changes 
   if( ellapsed_us > BITCHAT_BANDWIDTH_WINDOW_US )
   {
     ellapsed_us = BITCHAT_BANDWIDTH_WINDOW_US/32;
   }

   // calculate the weighted average for the bitrate
   auto total = avg_bits_per_usec*BITCHAT_BANDWIDTH_WINDOW_US + (8 * bytes_recv * ellapsed_us);
   avg_bits_per_usec = total / (BITCHAT_BANDWIDTH_WINDOW_US + ellapsed_us);

   // use the same weighting factors to weight the update to the average POW
   fc::bigint avg( &average_pow, sizeof(average_pow) );
   fc::bigint wsum = (avg * BITCHAT_BANDWIDTH_WINDOW_US + msg*ellapsed_us) 
                     / 
                     (BITCHAT_BANDWIDTH_WINDOW_US+ellapsed_us);

   fc::bigint tar( &target_pow, sizeof(target_pow) );
   if( avg_bits_per_usec >= target_bits_per_usec )
   {
      tar *= wsum * bigint( 990000ll);
      tar /= bigint(1000000ll);
   }
   else
   {
      // increase target (making it easier to get under)
      tar =  wsum * bigint(1010000ll); 
      tar /=        bigint(1000000ll);
   }

   std::vector<char> tarw(tar);
   std::vector<char> avgw(wsum);
   target_pow = pow_hash();

   memcpy( (char*)&target_pow + sizeof(pow_hash)-tarw.size(), 
            tarw.data(), tarw.size() );
   
   memcpy( (char*)&average_pow + sizeof(pow_hash)-avgw.size(), 
            avgw.data(), avgw.size() );

   return true;
}

} } 
