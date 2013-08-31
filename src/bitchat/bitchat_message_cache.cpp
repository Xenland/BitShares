#include <bts/bitchat/bitchat_message_cache.hpp>

namespace bts { namespace bitchat {

  namespace detail 
  {
     class message_cache_impl
     {
        public:

     };
  }

  message_cache::message_cache()
  :my ( new detail::message_cache_impl() )
  {
  }

  message_cache::~message_cache(){}

  void    message_cache::open( const fc::path& db_dir )
  {
  }

  void    message_cache::cache( const encrypted_message& msg )
  {

  }

  std::vector<fc::uint128> message_cache::get_inventory( const fc::time_point& start_time, const fc::time_point& end_time )
  {
     std::vector<fc::uint128> invent;

     return invent;
  }


} } // bts::bitchat
