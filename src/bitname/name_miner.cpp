#include <bts/bitname/name_miner.hpp>
#include <bts/bitname/name_hash.hpp>
#include <bts/config.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace bitname {

  namespace detail 
  {
    class name_miner_impl
    {
      public:
        name_miner_impl()
        :_callback_thread( fc::thread::current() ),
         _del(nullptr),
         _cur_effort(DEFAULT_MINING_EFFORT_PERCENT/100.0)
         {
            memset( min_name_pow.data, 0xff, sizeof(min_name_pow) );
            min_name_pow.data[0] = 255 - MIN_NAME_DIFFICULTY;
         }

        fc::thread&           _callback_thread;

        name_miner_delegate*  _del;
        fc::thread            _threads[DEFAULT_MINING_THREADS];

        fc::future<void>      _mining_complete[DEFAULT_MINING_THREADS];

        float                 _cur_effort;
        name_block            _cur_block;

        uint64_t              _block_ver;

        mini_pow              name_pow_target;
        mini_pow              min_name_pow;

        /**
         *  Called from mining thread
         */
        void start_mining( name_block b, uint32_t thread_num, uint64_t ver )
        {
            // calculate temp buffer
            std::vector<char> buf = fc::raw::pack( (const name_header&)b );
            uint32_t* nonce = (uint32_t*)buf.data();
            uint32_t* ts    = nonce + 1;
            *nonce = thread_num;
            *ts    = fc::time_point::now().time_since_epoch().count() / 1000000;

            while( ver >= _block_ver )
            {
                for( uint32_t cnt = 0; cnt < 10000*(_cur_effort); ++cnt ) 
                {
                    nonce += DEFAULT_MINING_THREADS;
                    auto test_result = mini_pow_hash( buf.data(), buf.size() );

                    if( test_result < name_pow_target )
                    {
                       ++_block_ver; // signal other threads to stop
                       b.nonce   = *nonce;
                       b.utc_sec = *ts;
                       _callback_thread.async( [=](){ _del->found_name_block( b ); } );
                    }
                    if( ver < _block_ver ) return;
                }
                fc::usleep( fc::microseconds( 1000000 * (1-_cur_effort) ) );
            }
        }

        void start_new_block()
        {
           FC_ASSERT( _del != nullptr ); // no point in mining if there is no one to tell when we find the result

           _cur_block.mroot = _cur_block.calc_merkle_root();

           name_pow_target = to_mini_pow( to_bigint( _cur_block.calc_difficulty() ) / fc::bigint(10000) );
           if( name_pow_target < min_name_pow )
           {
              name_pow_target = min_name_pow;
           }

           auto next_blk = ++_block_ver;
           for( uint32_t i = 0; i < DEFAULT_MINING_THREADS; ++i )
           {
              auto b = _cur_block; // create a copy to pass to thread
              _mining_complete[i] = _threads[i].async( [b,i,this,next_blk](){ start_mining(b,i,next_blk); } );
           }
        }
    };
  }

  name_miner::name_miner() :my( new detail::name_miner_impl() ) {}
  name_miner::~name_miner(){}

  void name_miner::set_delegate(  name_miner_delegate* d )
  {
     my->_del = d;
  }


  void name_miner::start( float effort )
  {
    bool kickoff = my->_cur_effort <= 0;
    my->_cur_effort = effort;
    if( kickoff )
    {
       my->start_new_block();
    }
  }

  void name_miner::stop()
  {
    bool wait_stop = my->_cur_effort > 0;
    my->_cur_effort = 0;
    ++my->_block_ver;

    if( wait_stop )
    {
       for( uint32_t i = 0; i < DEFAULT_MINING_THREADS; ++i )
       {
          my->_mining_complete[i].wait();
       }
    }
  }


  void name_miner::set_prev( const mini_pow& p )
  {
      my->_cur_block.registered_names.clear();
      my->start_new_block();
  }

  void name_miner::add_name_trx( const name_trx& t )
  {
      my->_cur_block.registered_names.push_back(t);
      my->start_new_block();
  }

  void name_miner::set_name( const std::string& name, const fc::ecc::public_key& k )
  {
     FC_ASSERT( name.size() < 32 );
     my->_cur_block.name_hash = bts::bitname::name_hash(name);
  }



} } // bts::bitname
