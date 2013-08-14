#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/exception/exception.hpp>
#include <algorithm>

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
            :_window(window){}

            fc::time_point           _origin_time;
            fc::microseconds         _block_interval;
            uint32_t                 _window;
                                    
            fc::bigint               _cur_difficulty;
            fc::bigint               _next_difficulty;
            fc::bigint               _cur_time;

            std::vector<time_record> _records;
            std::vector<uint32_t>    _sorted_by_difficulty;
            std::vector<uint32_t>    _sorted_by_time;

            void sort_by_difficulty()
            {
               std::sort( _sorted_by_difficulty.begin(), _sorted_by_difficulty.end(),
                          [&]( uint32_t a, uint32_t b ) 
                          {
                             return _records[a].block_pow < _records[b].block_pow;
                          } );
            }
            void sort_by_time()
            {
               std::sort( _sorted_by_time.begin(), _sorted_by_time.end(),
                          [&]( uint32_t a, uint32_t b ) 
                          {
                             return _records[a].block_time < _records[b].block_time;
                          } );
            }
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
   if( my->_records.size() )
   {
      FC_ASSERT( my->_records.back().block_num + 1 == block_num );
   }
   my->_records.emplace_back( detail::time_record( block_num, block_time, std::move(block_proof_of_work) ) );
   my->_sorted_by_difficulty.push_back( my->_sorted_by_difficulty.size() );
   my->_sorted_by_time.push_back( my->_sorted_by_time.size() );

   my->sort_by_difficulty();
   my->sort_by_time();
}


/**
 *  pop everything after block_num
 */
void time_keeper::pop( uint32_t block_num )
{
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
   int64_t error_us   = (next_time() - (current_time()+my->_block_interval)).count();
   error_us *= 1000000; // add 6 dec precision
   int64_t error_percent = error_us /= my->_block_interval.count(); // with 6 dec prec
   
   error_percent *= 2; // multiple how much we are off by 2 so we can bring the
                       // average back in line.
                       
   // if error percent is positive, the network is too fast... slow it down
   // by increasing difficulty, ie div by 1+abs(error_percent)
   if( error_percent > 0 )
   {
     error_percent += 1000000; // aka 1.000000
     auto tmp = my->_cur_difficulty * fc::bigint(1000000);
     my->_next_difficulty = tmp / fc::bigint(error_percent);
     return my->_next_difficulty;
   }

   // if error_percent is negitive, the network is too slow,  speed it up
   // by reducing difficulty (multiply threshold) by error_percent
   auto tmp = my->_cur_difficulty * fc::bigint(-error_percent);
   my->_next_difficulty = tmp / fc::bigint(1000000);

   return my->_next_difficulty;
}
         

/**
 * Return the current difficulty level as the
 * average of all blocks in the window.
 */
fc::bigint time_keeper::current_difficuty()const
{
   FC_ASSERT( my->_records.size() != 0 );
   auto idx = my->_sorted_by_difficulty[ my->_sorted_by_difficulty.size() / 2 ];
   return my->_records[idx].block_pow;
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

   auto idx = my->_sorted_by_time[ my->_sorted_by_time.size() / 2 ];
   auto base_time =  my->_records[idx].block_time;
   base_time += fc::microseconds(my->_block_interval.count() * ((my->_window / 2)-1));
   return base_time;
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
