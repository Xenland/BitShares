#include <bts/peer/peer_db.hpp>

namespace bts { namespace peer {

  namespace detail 
  {
    class peer_db_impl
    {
      public:

    };

  }
  
  peer_db::peer_db()
  :my( new detail::peer_db_impl() )
  {
  }


  peer_db::~peer_db(){}
  void peer_db::open( const fc::path& dbdir, bool create )
  {
  }
  void peer_db::close()
  {
  }

  /**
   *  On a fresh startup the entire database is 'out of date' and therefore
   *  all hosts should be reset to 1 hour old so they can expire if we have
   *  not heard about them in 2 hours.
   */
  void                peer_db::reset_ages( const fc::time_point_sec& s )
  {
  }

  void                peer_db::store( const host& r )
  {
  }

  host                peer_db::fetch_record( const fc::ip::endpoint& ep )
  {
    return host();
  }

  /**  Removes a host from the DB, presumably because we attempted to connect 
   * to it and were unable to.  
   *
   *   TODO: how do we prevent purging the entire DB if the internet goes down
   *   and no hosts are reachable?   Perhaps only perge nodes if we are successfully
   *   connected to other nodes.
   */
  void                peer_db::remove( const fc::ip::endpoint& ep )
  {
  }

  void                peer_db::add_channel( const fc::ip::endpoint& e, const channel_id& c )
  {
  }

  void                peer_db::remove_channel( const fc::ip::endpoint& e, const channel_id& c )
  {
  }


  std::vector<host>   peer_db::fetch_hosts( const channel_id& c )
  {
    return std::vector<host>();
  }

  void                peer_db::purge_old( const fc::time_point& age )
  {
  }


} }
