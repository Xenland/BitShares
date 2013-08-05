#pragma once
#include <bts/network/channel_id.hpp>
#include <bts/network/feature_id.hpp>
#include <fc/network/ip.hpp>
#include <fc/time.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/reflect/reflect.hpp>

namespace bts { namespace peer {

  using network::channel_id;

  /**
   *  Tracks a contact address and the last time it was heard about.
   */
  struct host
  {
     host( const fc::ip::endpoint& ep, const network::channel_id& c, const fc::time_point_sec& = fc::time_point::now() );
     host(){}

     fc::ip::endpoint                 ep;
     fc::time_point_sec               last_com;

     /**
      *   hosts that we hear about frequently and that never leave our DB are probably long lived nodes
      *   and thus good candidates for seed hosts... 
      */
     fc::time_point_sec               first_com; // set when a host is added to the database
     std::vector<network::channel_id> channels;  // TODO: defien size limit for unpacking these
     std::vector< fc::enum_type<fc::unsigned_int,network::feature_id > > features;  // TODO: define size limit for unpacking these
  };

} }  // bts::peer

FC_REFLECT( bts::peer::host, (ep)(last_com)(first_com)(channels)(features) )
