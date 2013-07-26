#include <bts/db/peer_ram.hpp>
#include <unordered_map>
#include <fc/exception/exception.hpp>
#include <fc/log/logger.hpp>
#include <map>

namespace bts { namespace db {


   typedef std::set<fc::ip::endpoint>                        ep_set_t;
   typedef std::map<network::channel_id, ep_set_t>           channel_map_t;
   typedef std::unordered_map<std::string, ep_set_t>         feature_map_t;

   struct peer_index
   {
       std::map<fc::ip::endpoint, peer::record> records;
       ep_set_t                                active_index;
       channel_map_t                           channel_index;
       feature_map_t                           feature_index;
   };


   namespace detail
   {
      class peer_ram_impl
      {
         public:
          peer_index  idx;

          void clear_channel_index( const fc::ip::endpoint& ep )
          {
              for( auto itr = idx.channel_index.begin();
                        itr != idx.channel_index.end();
                        ++itr )
              {
                 itr->second.erase( ep );
              }
          }
          void clear_feature_index( const fc::ip::endpoint& ep )
          {
              for( auto itr = idx.feature_index.begin();
                        itr != idx.feature_index.end();
                        ++itr )
              {
                 itr->second.erase( ep );
              }
          }
      };
   }

   peer_ram::peer_ram()
   :my( new detail::peer_ram_impl() )
   {
   }
   peer_ram::~peer_ram()
   {}

   void          peer_ram::store( const record& r )
   {
       // remove any old info for this contact from our index
       remove(r.contact);
       // update the record
       my->idx.records[r.contact] = r; 

       // re-index it
       for( auto itr = r.channels.begin(); itr != r.channels.end(); ++itr )
          my->idx.channel_index[*itr].insert(r.contact);

       for( auto itr = r.features.begin(); itr != r.features.end(); ++itr )
          my->idx.feature_index[*itr].insert(r.contact);
   }

   std::vector<peer::record>  peer_ram::get_all_peers()const 
   {
      std::vector<peer::record> r;
      r.reserve( my->idx.records.size() );
      for( auto itr = my->idx.records.begin(); itr != my->idx.records.end(); ++itr )
      {
         r.push_back(itr->second);
      }
      return r;
   }

   std::vector<peer::record>  peer_ram::get_peers_on_channel( const network::channel_id& cid )const
   {
      std::vector<peer::record> r;
      auto chan_itr = my->idx.channel_index.find(cid);
      if( chan_itr == my->idx.channel_index.end() ) return r;

      r.reserve( chan_itr->second.size() );
      for( auto itr = chan_itr->second.begin(); itr != chan_itr->second.end(); ++itr )
      {
         r.push_back( fetch( *itr ) );
      }
      return r;
   }

   std::vector<peer::record>  peer_ram::get_peers_with_feature( const std::string& s )const 
   {
      std::vector<peer::record> r;
      auto feature_itr = my->idx.feature_index.find(s);
      if( feature_itr == my->idx.feature_index.end() ) return r;

      r.reserve( feature_itr->second.size() );
      for( auto itr = feature_itr->second.begin(); itr != feature_itr->second.end(); ++itr )
      {
         r.push_back( fetch( *itr ) );
      }
      return r;
   }

   peer::record        peer_ram::fetch( const fc::ip::endpoint& ep )const
   {
       auto itr = my->idx.records.find(ep);
       if( itr != my->idx.records.end() )
       {
          ilog( "fetch active ${ep}", ("ep", itr->second.contact ) );
          return itr->second;
       }
       FC_THROW_EXCEPTION( key_not_found_exception, "No peer record for ${ep}", ("ep",ep) );
   }

   void          peer_ram::remove( const fc::ip::endpoint& ep )
   {
       my->clear_channel_index( ep );
       my->clear_feature_index( ep );
       my->idx.active_index.erase( ep );
       my->idx.records.erase(ep);
   }

   void peer_ram::update_last_com( const fc::ip::endpoint& ep, const fc::time_point& tp )
   {
       auto itr = my->idx.records.find(ep);
       if( itr != my->idx.records.end() )
       {
          itr->second.last_com = tp;    
          return;
       }
       FC_THROW_EXCEPTION( key_not_found_exception, "No peer record for ${ep}", ("ep",ep) );
   }


} } // bts::db
