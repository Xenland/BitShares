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
         time_record():block_num(0),time_error(0){}
         time_record( uint32_t num, fc::time_point blk_t, fc::bigint pow, uint64_t error_us )
         :block_num(num),block_time(blk_t),block_pow( std::move(pow) ),time_error(error_us){}

         uint32_t        block_num;
         fc::time_point  block_time;
         fc::bigint      block_pow;
         int64_t         time_error;
      };

      class time_keeper_impl
      {
         public:
            time_keeper_impl( uint32_t window )
            :_window(window){}

            fc::time_point           _origin_time;
            fc::microseconds         _block_interval;
            uint32_t                 _window;

            fc::bigint               _cur_difficulty;
            fc::bigint               _next_difficulty;
            fc::time_point           _cur_time;

            std::deque<time_record>  _records;
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
   if( my->_records.size() == 0 )
   {
     my->_cur_time        = block_time;
     my->_cur_difficulty  = block_proof_of_work;
     my->_next_difficulty = block_proof_of_work;
     auto expected_time =  my->_origin_time + fc::microseconds(block_num * my->_block_interval.count());
     int64_t error_us = (block_time - expected_time).count();
     my->_records.emplace_back( detail::time_record( block_num, block_time, block_proof_of_work, error_us ) );
     return;
   }

   FC_ASSERT( my->_records.back().block_num + 1 == block_num );
   FC_ASSERT( block_proof_of_work <= my->_next_difficulty        ); // we set a difficulty for a reason!
   FC_ASSERT( block_time > (my->_cur_time - fc::seconds(60*60) ) ); // 1 hr grace.. 

   int64_t error_us = (block_time - next_time()).count();
   //ilog( "${n}, ${t}, ${e} ${p}", ("n",block_num)("t",block_time)("p",std::string(block_proof_of_work))("e",error_us) );
   my->_records.emplace_back( detail::time_record( block_num, block_time, block_proof_of_work, error_us ) );
   if( my->_records.size() > my->_window ) 
   {
     my->_records.pop_front();
   }

   std::vector<int64_t>     errors;
   std::vector<int64_t>     index;

   index.reserve( my->_records.size() );
   errors.reserve( my->_records.size() );
   for( auto itr = my->_records.begin(); itr != my->_records.end(); ++itr )
   {
     errors.push_back( itr->time_error );
     index.push_back( index.size() );
   }
   uint32_t median_pos = my->_records.size() / 2;
   std::nth_element( index.begin(), index.begin()+median_pos, index.end(), 
      [&](int32_t a, int32_t b) { return my->_records[a].block_pow < my->_records[b].block_pow; } );

   std::nth_element( errors.begin(), errors.begin() + median_pos, errors.end() );
   int64_t median_error = errors[median_pos];

   // 1000 * error_percent (fixed point)
   int64_t error_percent = median_error * 1000000 / my->_block_interval.count();
   error_percent /= my->_window;
    
   my->_cur_time       = my->_origin_time + fc::microseconds(block_num * my->_block_interval.count()) + fc::microseconds(median_error);
   my->_cur_difficulty = my->_records[index[median_pos]].block_pow;
   
   if( error_percent < 0 ) // divided by  1 - error_percent
   {
      my->_next_difficulty = my->_cur_difficulty;
      my->_next_difficulty *= fc::bigint( 1000000 );
      my->_next_difficulty /= fc::bigint( 1000000 - error_percent );
   }
   else // multiply by 1 + error_percent
   {
      my->_next_difficulty = my->_cur_difficulty;
      my->_next_difficulty *= fc::bigint( 1000000 + error_percent );
      my->_next_difficulty /= 1000000;
   }
}


/**
 *  pop everything after block_num
 */
void time_keeper::pop( uint32_t block_num )
{
   FC_ASSERT( !"Not tested" );
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
   return my->_next_difficulty;
}
         

/**
 * Return the current difficulty level as the
 * average of all blocks in the window.
 */
fc::bigint time_keeper::current_difficulty()const
{
   return my->_cur_difficulty;
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
   return my->_cur_time;
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
