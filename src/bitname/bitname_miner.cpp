#include <bts/bitname/bitname_miner.hpp>
#include <bts/bitname/bitname_hash.hpp>
#include <bts/difficulty.hpp>
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
         _callback_del(nullptr),
         _cur_effort(0), //TODO: restore.. DEFAULT_MINING_EFFORT_PERCENT/100.0)
         _block_ver(0),
         _block_target(0),
         _name_trx_target(0),
         _min_name_trx_target(0)
         {
            fc::sha224 min_name_hash_target;
            // TODO: remove hardcoded 3
            memset( ((char*)&min_name_hash_target) + 3, 0xff, sizeof(fc::sha224)-3 );

            _name_trx_target     = difficulty( min_name_hash_target );
            _block_target        = _name_trx_target;
            _min_name_trx_target = _name_trx_target;

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
        name_miner_delegate*  _callback_del;

        fc::thread            _threads[DEFAULT_MINING_THREADS];

        fc::future<void>      _mining_complete[DEFAULT_MINING_THREADS];

        float                 _cur_effort;
        name_block            _cur_block;

        uint64_t              _block_ver; // incremented anytime block state changes
        uint64_t              _block_target;
        uint64_t              _name_trx_target;
        uint64_t              _min_name_trx_target;

        /**
         *  Called from mining thread
         */
        void start_mining( name_block b, uint32_t thread_num, uint64_t ver )
        {
          try { 
            ilog( "thread: ${t}  ver ${ver}", ("t",thread_num)("ver",ver) );
            if( b.name_hash == 0 ) return;

            uint16_t max_nonce(-1);
            
            while( ver >= _block_ver )
            {
               b.utc_sec = fc::time_point::now();
               for( uint32_t nonce = 0; nonce < max_nonce; nonce += DEFAULT_MINING_THREADS )
               {
                   b.nonce   = nonce;
               
                   uint64_t header_difficulty = b.difficulty();
                   if( header_difficulty > _name_trx_target )
                   {
                      uint64_t block_difficulty = b.block_difficulty();
                      if( block_difficulty > _block_target )
                      {
                         ++_block_ver;
                         _callback_thread.async( [=](){ _callback_del->found_name_block( b ); } );
                      }
                      else
                      {
                        name_trx ntrx = b; // avoid capturing the entire block!
                         _callback_thread.async( [=](){ _callback_del->found_name_trx( ntrx ); } );
                      }
                   }
                   // exit if the block has been updated
                   if( ver < _block_ver ) { return; }
               }
               do {
                   fc::usleep( fc::microseconds( 100 + 1000000 * (1-_cur_effort) ) );
               } while ( b.utc_sec == fc::time_point::now() );
            }
          }
          catch ( const fc::exception& e )
          {
            wlog( "${e}\n ${block}", ("e", e.to_detail_string() )("block",b) );
            // TODO: do something smart with this exception!
          }
        }

        void start_new_block()
        {
           ilog( "start_new_block" );
           FC_ASSERT( _callback_del != nullptr ); // no point in mining if there is no one to tell when we find the result

           _cur_block.trxs_hash = _cur_block.calc_trxs_hash();

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

  void name_miner::set_delegate(  name_miner_delegate* callback_del )
  {
     my->_callback_del = callback_del;
  }
  void name_miner::set_block_target( uint64_t block_target )
  {
     my->_block_target = block_target;
     // TODO: remove hard coded 10000... define why whe chose 10000... because it should
     // result keeping the trx registration rate in line.. with 10000 per block (about 1MB)
     // TODO: perhaps keep this limit lower until the hashing power grows enough...?
     my->_name_trx_target = std::max( my->_min_name_trx_target, my->_block_target / 10000 );
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

  void name_miner::add_name_trx( const name_header& t )
  {
      FC_ASSERT( t.prev == my->_cur_block.prev );
      my->_cur_block.name_trxs.push_back(t);
      my->start_new_block();
  }

  void name_miner::set_name_header( const name_header& name_trx_to_mine )
  {
      my->_cur_block = name_block(name_trx_to_mine);
      my->start_new_block();
  }



} } // bts::bitname
