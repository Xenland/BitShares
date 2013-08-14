#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/exception/exception.hpp>
#include <algorithm>
#include <deque>
#include <fc/log/logger.hpp>
#include <map>

namespace bts { namespace blockchain {

  namespace detail 
  { 
      struct time_record
      {
         time_record():block_num(0){}
         time_record( uint32_t num, fc::time_point blk_t, fc::bigint pow )
         :block_num(num),block_time(blk_t),block_pow( std::move(pow) ){}

         uint32_t        block_num;
         fc::time_point  block_time;
         fc::bigint      block_pow;
      };

      class time_keeper_impl
      {
         public:
            time_keeper_impl( uint32_t window )
            :_window(window),_first(0){}

            fc::time_point           _origin_time;
            fc::microseconds         _block_interval;
            uint32_t                 _window;

            uint32_t                 _first;
                                    
            fc::bigint               _cur_difficulty;
            fc::bigint               _next_difficulty;
            fc::bigint               _cur_time;

            std::deque<time_record>              _records;
            std::map<fc::bigint,uint32_t>        _sorted_by_difficulty;
            std::map<fc::time_point,uint32_t>    _sorted_by_time;
      };
  }

time_keeper::time_keeper( fc::time_point origin_time, 
             fc::microseconds block_interval, 
             uint32_t window )
:my( new detail::time_keeper_impl(window) )
{
   my->_origin_time    = origin_time;
   my->_block_interval = block_interval;
}

time_keeper::~time_keeper() { }


void time_keeper::push( uint32_t block_num, fc::time_point block_time, fc::bigint block_proof_of_work )
{
   //ilog( "${n}, ${t}, ${p}", ("n",block_num)("t",block_time)("p",std::string(block_proof_of_work)) );
   if( my->_records.size() )
   {
      FC_ASSERT( my->_records.back().block_num + 1 == block_num );
   }
   FC_ASSERT( my->_sorted_by_difficulty.size() == my->_records.size() );
   my->_records.emplace_back( detail::time_record( block_num, block_time, block_proof_of_work ) );
   my->_sorted_by_difficulty[block_proof_of_work] = block_num;
   my->_sorted_by_time[block_time] = block_num;
   FC_ASSERT( my->_sorted_by_difficulty.size() == my->_records.size() );
   FC_ASSERT( my->_sorted_by_time.size() == my->_records.size() );

   if( my->_records.size() > my->_window )
   {
       my->_sorted_by_time.erase( my->_records[0].block_time );
       my->_sorted_by_difficulty.erase( my->_records[0].block_pow );
       my->_first = my->_records.front().block_num+1;
       my->_records.pop_front();
   FC_ASSERT( my->_sorted_by_difficulty.size() == my->_records.size() );
   FC_ASSERT( my->_sorted_by_time.size() == my->_records.size() );
   }
}


/**
 *  pop everything after block_num
 */
void time_keeper::pop( uint32_t block_num )
{
   FC_ASSERT( !"Not tested" );
   /*
   while( my->_records.size() ) 
   {
     if( my->_records.back().block_num > block_num )
     {
       my->_records.pop_back();
       std::remove( my->_sorted_by_difficulty.begin(), my->_sorted_by_difficulty.end(),
                    my->_sorted_by_difficulty.size() - 1);
       my->_sorted_by_difficulty.pop_back();
          
       std::remove( my->_sorted_by_time.begin(), my->_sorted_by_time.end(),
                    my->_sorted_by_time.size() - 1);
       my->_sorted_by_time.pop_back();

       my->sort_by_difficulty();
       my->sort_by_time();
     }
     else
     {
       return;
     }
   }
   */
}


uint32_t time_keeper::next_block_num()const
{
  if( my->_records.size() == 0 ) return 0;
  return my->_records.back().block_num + 1;
}


/**
 *  Calculate the difficulty for the next block.
 */
fc::bigint time_keeper::next_difficulty()const
{
   FC_ASSERT( my->_sorted_by_difficulty.size() == my->_records.size() );
   //ilog( "next time: ${error}", ("error", next_time()) );
   //ilog( "cur  time: ${error}", ("error", current_time()) );
   int64_t error_sec   = (next_time() - (current_time()+my->_block_interval)).count()/1000000;
   //ilog( "error_us: ${error}", ("error", error_us) );
   error_sec *= 10000000; // add 6 dec precision
   int64_t error_percent = error_sec / (my->_block_interval.count()/1000000); // with 6 dec prec
   //ilog( "error_percent (6 dec): ${error}", ("error", error_percent) );

 //  ilog( "double error_percent (6 dec): ${error}", ("error", error_percent) );
   error_percent /= (my->_window *2); // keep things more stable

   my->_cur_difficulty = current_difficulty();

   // if error percent is positive, the network is too fast... slow it down
   // by increasing difficulty, ie div by 1+abs(error_percent)
   if( error_percent > 0 )
   {
     error_percent += 10000000; // aka 1.000000
     auto tmp = my->_cur_difficulty * fc::bigint(10000000);
     my->_next_difficulty = tmp / fc::bigint(error_percent);
     return my->_next_difficulty;
   }

   // if error_percent is negitive, the network is too slow,  speed it up
   // by reducing difficulty (multiply threshold) by error_percent
   error_percent *= -1;
   error_percent += 10000000; // aka 1.000000
   auto tmp = my->_cur_difficulty * fc::bigint(error_percent);
   my->_next_difficulty = tmp / fc::bigint(10000000);

   return my->_next_difficulty;
}
         

/**
 * Return the current difficulty level as the
 * average of all blocks in the window.
 */
fc::bigint time_keeper::current_difficulty()const
{
   FC_ASSERT( my->_sorted_by_difficulty.size() == my->_records.size() );
   FC_ASSERT( my->_records.size() != 0 );
   //auto idx = my->_sorted_by_difficulty[ my->_sorted_by_difficulty.size() / 2 ];
   //return my->_records[idx-my->_first].block_pow;
 //  std::vector<std::pair<fc::bigint,uint32_t>> sorted( my->_sorted_by_difficulty.begin(), my->_sorted_by_difficulty.end() );
   auto median =  my->_sorted_by_difficulty.begin();
   std::advance( median, my->_sorted_by_difficulty.size()/2 );
   return median->first;
  // auto idx = sorted[ sorted.size() / 2 ];
}

/**
 *  Estimate the current time based only on the
 *  block history and assuming that the most recent
 *  block is included.  This is calculated as the
 *  median of the time records + half the time
 *  represented by the window.  The current time
 *  should represent the 'expected' time of the current
 *  head, which is different than the time reported 
 *  by the current head.
 */
fc::time_point time_keeper::current_time()const
{
   FC_ASSERT( my->_records.size() != 0 );
   if( my->_records.size() <= 2 ) return my->_records.back().block_time;
   auto median =  my->_sorted_by_time.begin();
   std::advance( median, my->_records.size()/2 );
   auto base_time = median->first;
   base_time += fc::microseconds(my->_block_interval.count() * ((my->_records.size() / 2)-1));
   return base_time;

   /*
   uint64_t sum = 0;
   for( uint32_t i = 0; i < 8; ++i )
   {
      ++median;
      sum += (median->first-base_time).count();
   }
   base_time += fc::microseconds( sum / 8 );
   */
}

/**
 *  The next time is always a multiple of the block interval because
 *  the goal is to keep the long-term average rate of block production
 *  such that the expected time of the next block is on the
 *  target interval.
 *
 */
fc::time_point time_keeper::next_time()const
{
  return my->_origin_time + fc::microseconds(next_block_num() * my->_block_interval.count());
}

} } // bts::blockchain
