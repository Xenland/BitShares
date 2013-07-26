#pragma once

namespace bts { namespace network {
  
  /**
   *  Different nodes may support different features that are not necessarily
   *  associated with channels.  Feature IDs allow us to identify these nodes.
   */
  enum feature_id 
  {
     unspecified = 0,
     kad         = 1 /** node participates in a KAD hash table **/
  };

} } 

FC_REFLECT_ENUM( bts::network::feature_id, (unspecified)(kad) )
