#pragma once
#include <bts/peer/peer_channel.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace blockchain {

  namespace detail { class blockchain_client_impl; }

  class blockchain_client
  {
    public:
      struct config
      {
          enum chan_name 
          { 
            bitshares_test_chan = 0, 
            bitshares_chan = 1
          }; 

          config()
          :chan_num(bitshares_test_chan){}

          fc::path     data_dir;
          chan_name    chan_num;
      };

      blockchain_client( const peer::peer_channel_ptr& peers );
      ~blockchain_client();

      void configure( const config& cfg );

    private:
      std::unique_ptr<detail::blockchain_client_impl> my;
  };

} }  // namespace bts::blockchain

FC_REFLECT_ENUM( bts::blockchain::blockchain_client::config::chan_name, (bitshares_test_chan)(bitshares_chan) )
FC_REFLECT( bts::blockchain::blockchain_client::config, (data_dir)(chan_num) )
