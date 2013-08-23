#include <bts/blockchain/blockchain_time_keeper.hpp>
#include <bts/config.hpp>
#include <fc/exception/exception.hpp>
#include <algorithm>
#include <deque>
#include <fc/log/logger.hpp>
#include <map>

#include <fc/uint128.hpp>

namespace bts { namespace blockchain {

  namespace detail 
  { 
      struct time_record
      {
         time_record():block_num(0),time_error_sec(0){}
         time_record( uint32_t num, fc::time_point blk_t, uint64_t difficulty, uint64_t error_sec )
         :block_num(num),block_time(blk_t),block_difficulty( difficulty ),time_error_sec(error_sec){}

         uint32_t        block_num;
         fc::time_point  block_time;
         uint64_t        block_difficulty;
         int32_t         time_error_sec;
      };

      class time_keeper_impl
      {
         public:
            time_keeper_impl()
            :_window(0),_interval_sec(0),_median_time_error_sec(0){}

            fc::time_point           _origin_time;
            fc::microseconds         _block_interval;
            uint32_t                 _window;

            uint64_t                 _cur_difficulty;
            uint64_t                 _next_difficulty;
            fc::time_point           _cur_time;

            std::deque<time_record>  _records;

            int64_t                  _interval_sec;
            int64_t                  _median_time_error_sec;
            int64_t                  _median_interval_sec;
            int64_t                  _target_interval_sec;

            void update_stats()
            {
               std::vector<uint32_t> index(_records.size());
               for( uint32_t i = 0; i < _records.size(); ++i ) { index[i] = i; }
               update_current_time(index);
               update_current_difficulty(index);
               update_median_interval(index);
               update_next_difficulty();
            }

            fc::time_point expected_time( uint32_t block_num )
            {
               return _origin_time + fc::seconds(block_num * _interval_sec);
            }

            int64_t interval( uint32_t p )
            {
              if( p > 0 && p < _records.size() )
              {
                return (_records[p].block_time - _records[p-1].block_time).count()/1000000;
              }
              return _interval_sec;
            }

            void update_median_interval( std::vector<uint32_t>& index )
            {
                uint32_t median_pos = index.size() / 2;
                std::nth_element( index.begin(), index.begin()+median_pos, index.end(), 
                   [&](int32_t a, int32_t b) { return interval(a) < interval(b); } );
                _median_interval_sec = interval( index[median_pos] );
            }
           
            void update_current_difficulty( std::vector<uint32_t>& index )
            {
                uint32_t median_pos = index.size() / 2;
                std::nth_element( index.begin(), index.begin()+median_pos, index.end(), 
                   [&](int32_t a, int32_t b) { return _records[a].block_difficulty < _records[b].block_difficulty; } );
                _cur_difficulty = _records[index[median_pos]].block_difficulty;
            }

            void update_next_difficulty()
            {
                if( _median_interval_sec == 0 )
                {
                  _median_interval_sec = 1;
                }
                _target_interval_sec = _interval_sec;
                if( _median_time_error_sec > _interval_sec/64 )
                {
                   _target_interval_sec *= 630;
                   _target_interval_sec /= 640;
                }
                else if( _median_time_error_sec < -_interval_sec/64 )
                {
                   _target_interval_sec *= 640;
                   _target_interval_sec /= 630;
                }
                _next_difficulty = (_cur_difficulty * _target_interval_sec) / _median_interval_sec;
            }

            void update_current_time( std::vector<uint32_t>& index )
            {
                uint32_t median_pos = index.size() / 2;
                std::nth_element( index.begin(), index.begin()+median_pos, index.end(), 
                   [&](int32_t a, int32_t b) { return _records[a].time_error_sec < _records[b].time_error_sec; } );
                
                _median_time_error_sec = _records[index[median_pos]].time_error_sec;

                _cur_time = expected_time( head_block_num() ) + fc::seconds(_median_time_error_sec);
                //ilog( "expected time: ${time}       current time: ${cur}   error: ${err}",
                 //     ("time",expected_time(head_block_num()))( "cur", _cur_time )("err",_median_time_error_sec) );
            }

            uint32_t head_block_num()const
            {
               if( _records.size() == 0 ) return 0;
               return _records.back().block_num;
            }
      };
  }

time_keeper::time_keeper() 
:my( new detail::time_keeper_impl() )
{
}
int64_t time_keeper::current_time_error()const
{
  return my->_median_time_error_sec;
}

void time_keeper::configure( fc::time_point origin_time, fc::microseconds block_interval, uint32_t window )
{
   my->_origin_time    = origin_time;
   my->_block_interval = block_interval;
   my->_interval_sec   = block_interval.count() / 1000000;
   my->_target_interval_sec = my->_interval_sec;
   my->_window         = window;
}

time_keeper::~time_keeper() { }


void time_keeper::push_init( uint32_t block_num, fc::time_point block_time, uint64_t block_difficulty )
{
//   ilog( "records.size: ${s}", ("s", my->_records.size() ) );
    int64_t error_sec = (block_time - my->expected_time(block_num)).count() / 1000000;
    my->_records.emplace_back( detail::time_record( block_num, block_time, block_difficulty, error_sec ) );

    if( my->_records.size() > my->_window ) 
    {
      my->_records.pop_front();
    }
}

void time_keeper::init_stats()
{
    my->update_stats();
}

void time_keeper::push( uint32_t block_num, fc::time_point block_time, uint64_t block_difficulty )
{
   FC_ASSERT( my->_records.size() > 0 );
 //  ilog( "records.size: ${s}", ("s", my->_records.size() ) );
   FC_ASSERT( my->_records.back().block_num + 1 == block_num, "${block_num}", ("block_num", block_num)("records.back.blocknum",my->_records.back().block_num) );
   FC_ASSERT( block_difficulty >= my->_next_difficulty        ); // we set a difficulty for a reason!
   FC_ASSERT( block_time >= (my->_cur_time - fc::seconds(BLOCKCHAIN_TIMEKEEPER_MIN_BACK_SEC) ),
              "block_time: ${block_time} _cur_time: ${cur_time}", 
              ("block_time", block_time)("cur_time",my->_cur_time)); // 1 hr grace.. 
   //ilog( "${block}   ${time} init diff  ${diff}", ("block",block_num)("time",block_time)("diff",block_difficulty) );
   int64_t error_sec = (block_time - my->expected_time(block_num)).count() / 1000000;
   my->_records.emplace_back( detail::time_record( block_num, block_time, block_difficulty, error_sec ) );

   if( my->_records.size() > my->_window ) 
   {
     my->_records.pop_front();
   }
   my->update_stats();
   //ilog( "${block}   ${time} next diff  ${diff}", ("block",block_num)("time",block_time)("diff",next_difficulty()) );
}


/**
 *  pop everything after block_num
 */
void time_keeper::pop( uint32_t block_num )
{
   // TODO: this could probably be done more effeciently
   while( my->_records.size() && my->_records.back().block_num >= block_num )
   {
      my->_records.pop_back();
   }
}


uint32_t time_keeper::next_block_num()const
{
  return my->head_block_num() + 1;
}


/**
 *  Calculate the difficulty for the next block.
 */
uint64_t time_keeper::next_difficulty()const
{
   return my->_next_difficulty;
}
         

/**
 * Return the current difficulty level as the
 * average of all blocks in the window.
 */
uint64_t time_keeper::current_difficulty()const
{
   return my->_cur_difficulty;
}

/**
 *  This value returns an estimate of the current time
 *  based upon the blockchain.   
 */
fc::time_point time_keeper::current_time()const
{
   return my->_cur_time;
}

int64_t time_keeper::median_interval()const
{
  return my->_median_interval_sec;
}
int64_t time_keeper::target_interval()const
{
  return my->_target_interval_sec;
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
  return my->expected_time( next_block_num() );
}
fc::time_point time_keeper::expected_time( uint32_t block_num )const
{
  return my->expected_time(block_num);
}

} } // bts::blockchain
