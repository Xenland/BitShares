#pragma once
#include <bts/network/channel_id.hpp>
#include <fc/network/ip.hpp>
#include <fc/time.hpp>
#include <memory>
#include <set>
#include <vector>
#include <unordered_set>

namespace bts { namespace db
{
   /**
    *  @brief abstracts the interface to the peer database
    *
    *  There may be multiple backends for storing IP/Port/stats
    *  for various peers in the network.  This interface 
    *  abstracts that process.
    */
   class peer
   {
      public:
         virtual ~peer(){}

         struct record
         {
            record():failures(0){}

            fc::ip::endpoint                         contact;
            fc::time_point                           last_com;
            uint32_t                                 failures;
            std::set<network::channel_id>            channels;
            std::unordered_set<std::string>          features;
         };

         virtual void                 store( const record& r ) = 0;
         virtual void                 remove( const fc::ip::endpoint& ep ) = 0;
         virtual void                 update_last_com( const fc::ip::endpoint& ep, 
                                                       const fc::time_point& ) = 0;

         virtual record               fetch( const fc::ip::endpoint& ep )const = 0;
         virtual std::vector<record>  get_all_peers()const = 0;
         virtual std::vector<record>  get_peers_on_channel( const network::channel_id& cid )const = 0;
         virtual std::vector<record>  get_peers_with_feature( const std::string& s )const = 0;
      
   };
   typedef std::shared_ptr<peer> peer_ptr;


} } // namespace bts::db

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::db::peer::record, 
   (contact)
   (last_com)
   (failures)
   (channels)
   (features) )
