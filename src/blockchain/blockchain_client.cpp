#include <bts/blockchain/blockchain_client.hpp>
#include <bts/blockchain/blockchain_channel.hpp>
#include <bts/blockchain/blockchain_db.hpp>

#include <fc/reflect/variant.hpp>

namespace bts { namespace blockchain {

  namespace detail 
  { 
    class blockchain_client_impl
    {
      public:
        peer::peer_channel_ptr     _peers;
        blockchain_db_ptr          _chain_db;
        blockchain::channel_ptr    _chain_chan;
        blockchain_client::config  _config;
    };
  } // namespace detail

  blockchain_client::blockchain_client( const peer::peer_channel_ptr& peers )
  :my( new detail::blockchain_client_impl() )
  {
    my->_peers = peers;
    my->_chain_db = std::make_shared<blockchain_db>();
  }

  blockchain_client::~blockchain_client()
  {
  }

  void blockchain_client::configure( const config& aconfig )
  {
     my->_config = aconfig;
     my->_chain_db->open( my->_config.data_dir / fc::variant(my->_config.chan_num).as_string() / "chaindb", true );
     
     // TODO: init chain with gensis block if necessary

     // TODO: load public wallet

     // TODO: connect to private wallet

     // subscribe to the proper chain channel...  
  }

  extended_address blockchain_client::get_recv_address( const std::string& contact_label )
  {
    return extended_address();
  }

  void             blockchain_client::add_contact( const std::string& label, const extended_address& send_to_addr )const
  {
  }

  void             blockchain_client::transfer( uint64_t amount, asset::type unit, const std::string& to_contact_label )const
  {
  }

  
  asset            blockchain_client::get_balance( asset::type unit, uint32_t min_conf  )const
  {
     return asset();
  }

  asset            blockchain_client::get_trx_balance( const std::string& contact_label, uint32_t trx_num, uint32_t min_conf  )const
  {
     return asset();
  }

} } // namespace bts::blockchain
