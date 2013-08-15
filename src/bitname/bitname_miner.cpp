#include <bts/bitname/bitname_miner.hpp>
#include <bts/bitname/bitname_hash.hpp>
#include <bts/config.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/raw.hpp>

#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>

namespace bts { namespace bitname {

  namespace detail 
  {
    class name_miner_impl
    {
      public:
        name_miner_impl()
        :_callback_thread( fc::thread::current() ),
         _del(nullptr),
         _cur_effort(0) //DEFAULT_MINING_EFFORT_PERCENT/100.0)
         {
            memset( min_name_pow.data, 0xff, sizeof(min_name_pow) );
            min_name_pow.data[0] = 255 - MIN_NAME_DIFFICULTY;
            name_pow_target = 1000;

            if( DEFAULT_MINING_THREADS > 0 ) _threads[0].set_name( "bitname1" );
            if( DEFAULT_MINING_THREADS > 1 ) _threads[1].set_name( "bitname2" );
            if( DEFAULT_MINING_THREADS > 2 ) _threads[2].set_name( "bitname3" );
            if( DEFAULT_MINING_THREADS > 3 ) _threads[3].set_name( "bitname4" );
         }
        ~name_miner_impl()
        {
          _block_ver = -1;
          for( uint32_t i = 0; i < DEFAULT_MINING_THREADS; ++i )
          {
            _threads[i].quit();
          }
        }

        fc::thread&           _callback_thread;

        name_miner_delegate*  _del;
        fc::thread            _threads[DEFAULT_MINING_THREADS];

        fc::future<void>      _mining_complete[DEFAULT_MINING_THREADS];

        float                 _cur_effort;
        name_block            _cur_block;

        uint64_t              _block_ver;
        mini_pow              _block_target;

        uint64_t              name_pow_target;
        mini_pow              min_name_pow;

        /**
         *  Called from mining thread
         */
        void start_mining( name_block b, uint32_t thread_num, uint64_t ver )
        {
          try { 
            ilog( "thread: ${t}  ver ${ver}", ("t",thread_num)("ver",ver) );
            if( b.name_hash == 0 ) return;

            // calculate temp buffer
            std::vector<char> buf = fc::raw::pack( (const name_header&)b );
            uint32_t* nonce = (uint32_t*)buf.data();
            uint32_t* ts    = nonce + 1;
            *nonce = thread_num;
            *ts    = fc::time_point_sec(fc::time_point::now()).sec_since_epoch();

            while( ver >= _block_ver )
            {
                for( uint32_t cnt = 0; cnt < 100000*(_cur_effort); ++cnt ) 
                {
                    *nonce += DEFAULT_MINING_THREADS;
                    auto test_result = mini_pow_hash( buf.data(), buf.size() );

                    if( cnt % 1000 == 0 ) ilog( "test: ${r} >? ${tar}  ${id}  ${nonce}", 
                                    ("tar",name_pow_target)("r",mini_pow_difficulty(test_result))("id",test_result)("nonce",*nonce) );
                    if( mini_pow_difficulty(test_result) > name_pow_target )
                    {
                       ++_block_ver; // signal other threads to stop
                       b.nonce   = *nonce;
                       b.utc_sec = fc::time_point_sec( *ts );
                      // wlog( "found... ${b}", ("b",b) );
                       wlog( "test: ${r} >? ${tar}  ${id}  ${nonce}", 
                                    ("tar",name_pow_target)("r",mini_pow_difficulty(test_result))("id",test_result)("nonce",*nonce) );
                       _callback_thread.async( [=](){ _del->found_name_block( b ); } );
                    }
                    if( ver < _block_ver )
                    {
                      return;
                    }
                }
                fc::usleep( fc::microseconds( 100 + 1000000 * (1-_cur_effort) ) );
                *ts    = fc::time_point_sec(fc::time_point::now()).sec_since_epoch();
            }
          }
          catch ( const fc::exception& e )
          {
            wlog( "${e}", ("e", e.to_detail_string() ) );
            // TODO: do something smart with this exception!
          }
        }

        void start_new_block()
        {
           ilog( "start_new_block" );
           FC_ASSERT( _del != nullptr ); // no point in mining if there is no one to tell when we find the result

           _cur_block.mroot = _cur_block.calc_merkle_root();

           //uint64_t block_diff = _cur_block.calc_difficulty();
           //name_pow_target = mini_pow_difficulty(min_name_pow);

           auto next_blk = ++_block_ver;
           if( _cur_block.name_hash != 0 )
           {
              for( uint32_t i = 0; i < DEFAULT_MINING_THREADS; ++i )
              {
                 auto b = _cur_block; // create a copy to pass to thread
                 _mining_complete[i] = _threads[i].async( [b,i,this,next_blk](){ start_mining(b,i,next_blk); } );
              }
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
  void name_miner::set_block_target( const mini_pow& block_target )
  {
     my->_block_target = block_target;
     my->start_new_block();
  }

  void name_miner::start( float effort )
  {
    ilog( "${effort}", ("effort",effort) );
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
     my->_cur_block.key = k;
     my->start_new_block();
  }



} } // bts::bitname
