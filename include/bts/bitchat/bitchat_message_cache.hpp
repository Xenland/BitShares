#pragma once
#include <bts/bitchat/bitchat_private_message.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace bitchat {

  namespace detail { class message_cache_impl; }

  /**
   *  Caches encrypted messages for up to 1 month.
   *  Once 'full', messages can be bumped based upon difficulty with the least difficult going first.
   */
  class message_cache
  {
     public:
       message_cache();
       ~message_cache();

       void                     open( const fc::path& db_dir );

       void                     cache( const encrypted_message& msg );
       std::vector<fc::uint128> get_inventory( const fc::time_point& start_time, const fc::time_point& end_time );

     private:
       std::unique_ptr<detail::message_cache_impl> my;
  };



} } // bts::bitchat 
