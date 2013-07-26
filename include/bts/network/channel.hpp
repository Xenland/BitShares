#pragma once
#include <bts/network/connection.hpp>
#include <bts/network/message.hpp>

namespace bts { namespace network {

  class channel
  {
    public:
      virtual ~channel(){}

      virtual void handle_message( const connection_ptr&, 
                                   const message& m ) = 0;
  };

  typedef std::shared_ptr<channel> channel_ptr;

} }


