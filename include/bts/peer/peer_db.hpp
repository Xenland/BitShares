#pragma once
#include <bts/peer/peer_host.hpp>

namespace bts { namespace peer {

  using network::channel_id;


  /**
   *  Maintains a database of peers indexed by age, channels, and features. This
   *  index is persistant and enables a node to maintain a large set of potential
   *  bootstrap nodes for future connections and to quickly connect to new channels
   *  when they need to.
   */
  class peer_db
  {
     public:
      peer_db();
      ~peer_db();

      void open( const fc::path& dbdir, bool create = true );
      void close();

      /**
       *  On a fresh startup the entire database is 'out of date' and therefore
       *  all hosts should be reset to 1 hour old so they can expire if we have
       *  not heard about them in 2 hours.
       */
      void                reset_ages( const fc::time_point_sec& s );

      void                store( const host& r );
      host                fetch_record( const fc::ip::endpoint& ep );

      /**  Removes a host from the DB, presumably because we attempted to connect 
       * to it and were unable to.  
       *
       *   TODO: how do we prevent purging the entire DB if the internet goes down
       *   and no hosts are reachable?   Perhaps only perge nodes if we are successfully
       *   connected to other nodes.
       */
      void                remove( const fc::ip::endpoint& ep );

      void                add_channel( const fc::ip::endpoint& e, const channel_id& c );
      void                remove_channel( const fc::ip::endpoint& e, const channel_id& c );

      std::vector<host>   fetch_hosts( const channel_id& c, uint32_t limit = 10 );
      void                purge_old( const fc::time_point& age );

     private:
      std::unique_ptr<detail::peer_db_impl> my;

  }; // peer_db

} }  // bts::peer

FC_REFLECT( bts::peer::host, (ep)(last_com)(first_com)(channels)(features) )
