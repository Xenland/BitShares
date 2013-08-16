#include <bts/bitname/bitname_client.hpp>
#include <bts/bitname/bitname_channel.hpp>
#include <bts/bitname/bitname_miner.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/json.hpp>
#include <fc/thread/thread.hpp>
#include <fc/reflect/variant.hpp>
#include <bts/db/level_map.hpp>

#include <fc/log/logger.hpp>

namespace bts { namespace bitname {
  
  namespace detail 
  {
     class client_impl : public name_miner_delegate, public name_channel_delegate
     {
        public:
          client_impl():_delegate(nullptr){}

          virtual void found_name_block( const name_block& new_block )
          {
              // make sure new_block is not stale... 
              // check to see if it is good enough to solve the block, or just a trx
              // broadcast it accordingly... 
              // move on to the next name... 
              _chan->submit_block( new_block );
          }

          /**
           *   Called any time a new & valid name reg trx is received
           */
          virtual void pending_name_registration( const name_trx& pending_trx )
          {
              _miner.add_name_trx( pending_trx );
          }
          
          /**
           *   Called any time a new & valid name block is added to the
           *   chain or replaces the head block.
           */
          virtual void name_block_added( const name_block& new_block )
          {
              _miner.set_prev( new_block.id() );

              // TODO: for each name in pending-regs lookup how many
              // confirmations it has, any with 6 confirmations can
              // be removed from the pending state.
          }

          client_delegate*                                 _delegate;
          client::config                                   _config;
          name_channel_ptr                                 _chan;
          name_miner                                       _miner;

          /**
           *  Which ever pending reg has the least confirmations and
           *  is still valid will be the next chosen.
           */
          db::level_map<std::string,fc::ecc::public_key>   _pending_regs;
     };

  } // detail

  client::client( const peer::peer_channel_ptr& peer_ch )
  :my( new detail::client_impl() )
  {
      my->_chan = std::make_shared<bts::bitname::name_channel>(peer_ch);
      my->_chan->set_delegate( my.get() );
      my->_miner.set_delegate( my.get() );
  }
  client::~client(){}

  void client::set_delegate( client_delegate* my_del )
  {
    my->_delegate = my_del;
  }

  void client::configure( const client::config& client_config )
  {
     my->_config = client_config;
     if( !fc::exists( my->_config.data_dir / "bitname" ) )
     {
       fc::create_directories( my->_config.data_dir / "bitname" );
     }
     my->_pending_regs.open( my->_config.data_dir / "bitname" / "pending", true /*create*/);
     my->_miner.start( my->_config.max_mining_effort );
  }

  name_record client::lookup_name( const std::string& name )
  {
     try {
       name_header head = my->_chan->lookup_name( name );
       name_record rec;
       rec.last_update  = head.utc_sec;
       //rec.num_updates  = head.renewal;
       //rec.revoked      = !!head.cancel_sig;
       rec.name_hash    = fc::to_hex( (char*)&head.name_hash, sizeof( head.name_hash)  );
       rec.name         = name;
       rec.pub_key      = head.key;
       
       return rec;
     } FC_RETHROW_EXCEPTIONS( warn, "error looking up name '${name}'", ("name",name) );
  }

  name_record client::reverse_name_lookup( const fc::ecc::public_key& k )
  {
    FC_ASSERT( !"Not Implemented" );
  }

  /**
   *  This wrapper on fc:::ecc::public_key is exposed on this client API so that it may
   *  be called via JSON-RPC by 3rd party apps that don't have the crypto methods required
   *  to derive the public key from the digest & signature.
   */
  fc::ecc::public_key client::verify_signature( const fc::sha256& digest, const fc::ecc::compact_signature& sig )
  {
     return fc::ecc::public_key( sig, digest );
  }

  void client::register_name( const std::string& bitname_id, const fc::ecc::public_key& name_key)
  {
     /*
     my->_pending_regs.store( bitname_id, name_key );
     my->_miner.set_name( bitname_id, name_key );
     my->_miner.start( my->_config.max_mining_effort );
     */
  }

  std::map<std::string,fc::ecc::public_key>  client::pending_name_registrations()const
  {
     std::map<std::string,fc::ecc::public_key> pending_regs;
     // TODO: load from _pending_regs db
     return pending_regs;
  }
  void                                       client::cancel_name_registration( const std::string& bitname_id )
  {
     my->_pending_regs.remove( bitname_id );
  }

  fc::ecc::compact_signature client::sign( const fc::sha256& digest, const std::string& bitname_id )
  {
    FC_ASSERT( !"Not Implemented" );
  }

} } // bts::bitname
