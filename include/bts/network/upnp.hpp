#pragma once
#include <stdint.h>
#include <memory>

namespace bts { namespace network {

  namespace detail {  class upnp_service_impl; }

  class upnp_service 
  {
    public:
      upnp_service();
      ~upnp_service();

      void map_port( uint16_t p );
    private:
      std::unique_ptr<detail::upnp_service_impl> my;
  };

} }
