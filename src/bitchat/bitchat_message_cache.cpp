#include <bts/bitchat/bitchat_message_cache.hpp>
#include <bts/db/level_pod_map.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/interprocess/mmap_struct.hpp>
#include <bts/config.hpp>

struct age_index
{
  age_index(){}
  age_index( fc::time_point_sec ts, const fc::uint128& id )
  :timestamp(ts),message_id(id){}

  fc::time_point_sec timestamp;
  fc::uint128        message_id;
};
FC_REFLECT( age_index, (timestamp)(message_id) );

bool operator < ( const age_index& a, const age_index& b )
{
   return a.timestamp < b.timestamp ? true : a.message_id < b.message_id;
}
bool operator == ( const age_index& a, const age_index& b )
{
   return (a.timestamp == b.timestamp) && (a.message_id == b.message_id);
}


namespace bts { namespace bitchat {

  namespace detail 
  {
     class message_cache_impl
     {
        public:
          db::level_pod_map<fc::uint128, encrypted_message> _cache_by_id;
          db::level_pod_map<age_index,uint32_t>             _age_index; // TODO: convert to set, value not used
          fc::mmap_struct<size_t>                           _stats;

          void purge_old()
          { try {
             fc::time_point_sec expired = fc::time_point::now() - fc::seconds( BITCHAT_CACHE_WINDOW_SEC );
             auto itr = _age_index.begin();
             while( itr.valid() )
             {
                auto key = itr.key();
                if( key.timestamp < expired )
                {
                  _age_index.remove(key);
                  auto msg = _cache_by_id.fetch(key.message_id);
                  *_stats -= msg.data.size();
                  _cache_by_id.remove(key.message_id);
                }
                ++itr;
             }
          } FC_RETHROW_EXCEPTIONS( warn, "" ) }

          void purge_easy( fc::uint128 target, size_t required_space )
          { try {
             fc::uint128 key; 
             encrypted_message val;
             while( required_space > 0 && _cache_by_id.last( key, val ) )
             {
               if( key > target )
               {
                  *_stats        -= val.data.size();
                  required_space -= val.data.size();
                  _cache_by_id.remove(key);
                  _age_index.remove( age_index( val.timestamp, key ) );
               }
             }
          } FC_RETHROW_EXCEPTIONS( warn, "", ("target",target)("required_space",required_space) ) }
     };
  }

  message_cache::message_cache()
  :my ( new detail::message_cache_impl() )
  {
  }

  message_cache::~message_cache(){}

  void    message_cache::open( const fc::path& db_dir )
  { try {
       fc::create_directories( db_dir / "message_cache" );
       my->_cache_by_id.open( db_dir / "message_cache" / "by_id"  );
       my->_age_index.open( db_dir / "message_cache" / "age_index"  );
       my->_stats.open( db_dir / "message_cache" / "stats", true ); 


       my->purge_old();
  } FC_RETHROW_EXCEPTIONS( warn, "", ("db_dir",db_dir) ) }

  void    message_cache::cache( const encrypted_message& msg )
  {
     my->purge_old();
     auto      id     = msg.id();

     if( (*my->_stats + msg.data.size() ) > BITCHAT_CHANNEL_SIZE )
     {
       my->purge_easy( id, msg.data.size() );
     }

     if( (*my->_stats + msg.data.size()) > BITCHAT_CHANNEL_SIZE )
     {
       return;  // not difficult enough to bother storing
     }

     my->_cache_by_id.store( id, msg );
     my->_age_index.store( age_index(msg.timestamp, id), 0  );
     *my->_stats += msg.data.size();
  }

  std::vector<fc::uint128> message_cache::get_inventory( const fc::time_point& start_time, const fc::time_point& end_time )
  {
     fc::time_point_sec endtime(end_time);

     std::vector<fc::uint128> invent;
     auto itr = my->_age_index.lower_bound( age_index(start_time,fc::uint128()) );
     while( itr.valid() )
     {
       auto key = itr.key();
       if( key.timestamp > endtime ) 
       {
          return invent;
       }
       invent.push_back( itr.key().message_id );
       ++itr;
     }
     return invent;
  }


} } // bts::bitchat
