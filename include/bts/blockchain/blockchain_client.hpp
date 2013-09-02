#pragma once
#include <bts/peer/peer_channel.hpp>
#include <bts/extended_address.hpp>
#include <bts/blockchain/asset.hpp>
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

      extended_address get_recv_address( const std::string& contact_label );
      void             add_contact( const std::string& label, const extended_address& send_to_addr )const;

      void             transfer( uint64_t amount, asset::type unit, const std::string& to_contact_label )const;

      /** buy $amount of $base_unit with $quote_unit at or above price_per_unit */
      void             bid( uint64_t amount, asset::type quote_unit, asset::type base_unit, double price_per_unit );

      /** sell $amount of $base_unit for $quote_unit at or above price_per_unit */
      void             ask( uint64_t amount, asset::type quote_unit, asset::type base_unit, double price_per_unit );

      /** sell $amount of $quote_unit at or above price_per_unit where base_unit is BTS*/
      void             short_sell( uint64_t amount, asset::type quote_unit, double price_per_unit );

      /** assuming you have a positive balance of amount/unit in your wallet, this will cover your
       * positions with the least margin first and work their way up to the positions with the most
       * margin.
       */
      void             cover( uint64_t amount, asset::type unit );
      
      asset            get_balance( asset::type unit, uint32_t min_conf = 1  )const;
      asset            get_trx_balance( const std::string& contact_label, uint32_t trx_num, uint32_t min_conf = 1  )const;

    private:
      std::unique_ptr<detail::blockchain_client_impl> my;
  };

} }  // namespace bts::blockchain

FC_REFLECT_ENUM( bts::blockchain::blockchain_client::config::chan_name, (bitshares_test_chan)(bitshares_chan) )
FC_REFLECT( bts::blockchain::blockchain_client::config, (data_dir)(chan_num) )
