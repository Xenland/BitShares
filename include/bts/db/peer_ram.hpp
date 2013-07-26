#pragma once
#include <bts/db/peer.hpp>

namespace bts { namespace db {
  
   namespace detail { class peer_ram_impl; }

   /**
    *  @brief manages the peer database in memory, nothing
    *  persists after relaunch.
    */
   class peer_ram : public peer 
   {
       public:
         peer_ram();
         virtual  ~peer_ram();

         virtual void                 store( const record& r );
         virtual record               fetch( const fc::ip::endpoint& ep )const;
         virtual void                 remove( const fc::ip::endpoint& ep );
         virtual void                 update_last_com( const fc::ip::endpoint& ep, 
                                                       const fc::time_point& );

         virtual std::vector<peer::record>  get_all_peers()const;
         virtual std::vector<peer::record>  get_peers_on_channel( const network::channel_id& cid )const;
         virtual std::vector<peer::record>  get_peers_with_feature( const std::string& s )const;
       private:
         std::unique_ptr<detail::peer_ram_impl> my;
   };

   typedef std::shared_ptr<peer_ram> peer_ram_ptr;

} }
