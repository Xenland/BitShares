#include <bts/bitname/bitname_client.hpp>
#include <bts/bitname/bitname_channel.hpp>
#include <bts/bitname/bitname_miner.hpp>
#include <bts/bitname/bitname_hash.hpp>
#include <fc/crypto/hex.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/json.hpp>
#include <fc/thread/thread.hpp>
#include <fc/reflect/variant.hpp>
#include <bts/db/level_map.hpp>

#include <unordered_map>

#include <fc/log/logger.hpp>

namespace bts { namespace bitname {
  
  namespace detail 
  {
     class client_impl : public name_miner_delegate, public name_channel_delegate
     {
        public:
          client_impl():_self(nullptr),_delegate(nullptr){}

          virtual void found_name_block( const name_block& new_block )
          {
              // make sure new_block is not stale... 
              // check to see if it is good enough to solve the block, or just a trx
              // broadcast it accordingly... 
              // move on to the next name... 
              try {
                 ilog( "found block ${id}  ${difficulty} \n${s}", ("id",new_block.id())("difficulty",new_block.difficulty())("s", fc::json::to_pretty_string(new_block) ) );
                 _chan->submit_block( new_block );
              } 
              catch ( const fc::exception& e )
              {
                 wlog( "\n${e}", ("e",e.to_detail_string()) );
                 fc::usleep( fc::seconds(3) );
              }
              start_mining();
          }

          /**
           *   Called any time a new & valid name reg trx is received
           */
          virtual void pending_name_trx( const name_header& pending_trx )
          {
              ilog( "pending name trx\n${s}", ("s", fc::json::to_pretty_string(pending_trx) ) );
              _miner.add_name_trx( pending_trx );
          }
          
          /**
           *   Called any time a new & valid name block is added to the
           *   chain or replaces the head block.
           */
          virtual void name_block_added( const name_block& new_block )
          {
             ilog( "name block added\n${s}", ("s", fc::json::to_pretty_string(new_block) ) );
             // a new block has been added, restart mining 
             start_mining();
          }

          void start_mining()
          {
             fc::optional<name_record> min_repute_record;
             for( auto itr = _names_to_mine.begin(); itr != _names_to_mine.end(); /*don't inc cause we may remove*/ )
             {
                ilog( "lookup name ${n}", ("n",itr->first) );
                fc::optional<name_record> name_rec = _self->lookup_name( itr->first );
                if( name_rec.valid() )
                {
                   /**
                    *  We found the name in the db already, perform various checks and
                    *  raise any errors if we are unable to mine a name.
                    */
                   try {
                     FC_ASSERT( name_rec->pub_key == itr->second, "attempt to renew name with different public key" );
                     FC_ASSERT( name_rec->revoked == false );
                     if( !min_repute_record.valid() )
                     {
                       min_repute_record = name_rec;
                     } 
                     else if( name_rec->repute < min_repute_record->repute )
                     {
                        min_repute_record = name_rec;
                     }
                     else if( name_rec->repute == min_repute_record->repute )
                     {
                        if( name_rec->last_update < min_repute_record->last_update )
                        {
                            min_repute_record = name_rec;
                        }
                     }
                   } 
                   catch ( const fc::exception& e )
                   {
                       wlog( "${e}", ("e", e.to_detail_string() ) );
                       itr = _names_to_mine.erase(itr);
                       continue;
                   }
                }
                else
                {
                   /** we found an unregistered name, that is as low as it gets so
                    *  we can go ahead and start mining it 
                    */
                   name_header new_name;
                   new_name.name_hash     = name_hash( itr->first );
                   new_name.key           = itr->second;
                   new_name.repute_points = 1;
                   new_name.age           = _chan->get_head_block_number() + 1;
                   new_name.prev          = _chan->get_head_block_id();
                   _miner.set_name_header(new_name);
                   auto pending_trxs = _chan->get_pending_name_trxs();
                   ilog( "start new name reg: num pending ${n}", ("n",pending_trxs.size())  );
                   for( auto itr = pending_trxs.begin(); itr != pending_trxs.end(); ++itr )
                   {
                      _miner.add_name_trx(*itr);
                   }
                   _miner.start(); // TODO: set mining effort
                   return;
                }
                ++itr;
             }
             if( min_repute_record.valid() )
             {
                ilog( "update name reg" );
                name_header renew_name;
                renew_name.name_hash       = min_repute_record->get_name_hash();
                renew_name.key             = min_repute_record->pub_key;
                renew_name.age             = min_repute_record->age;
                // TODO: handle repute + num_trxs in case last renewal was a block header
                renew_name.repute_points   = min_repute_record->repute + 1;
                renew_name.prev            = _chan->get_head_block_id();
                _miner.set_name_header( renew_name );

                auto pending_trxs = _chan->get_pending_name_trxs();
                ilog( "start new name reg: num pending ${n}", ("n",pending_trxs.size())  );
                for( auto itr = pending_trxs.begin(); itr != pending_trxs.end(); ++itr )
                {
                   _miner.add_name_trx(*itr);
                }
                _miner.start(); // TODO: set mining effort
             }
             else
             {
                _miner.stop();
             }
          }

          client*                                          _self;
          client_delegate*                                 _delegate;
          client::config                                   _config;
          name_channel_ptr                                 _chan;
          name_miner                                       _miner;

          /**
           *  Tracks all of the active names that are being mined.  
           */
          std::unordered_map<std::string,fc::ecc::public_key_data> _names_to_mine;
     };

  } // detail

  client::client( const peer::peer_channel_ptr& peer_ch )
  :my( new detail::client_impl() )
  {
      my->_self = this;
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
  { try {

     my->_config = client_config;
      
     if( !fc::exists( my->_config.data_dir / "bitname" ) )
     {
       fc::create_directories( my->_config.data_dir / "bitname" );
     }
     bitname::name_channel::config chan_config;
     chan_config.name_db_dir =  my->_config.data_dir / "bitname" / "channel";
     my->_chan->configure( chan_config );

  } FC_RETHROW_EXCEPTIONS( warn, "error configuring bitname client", ("config",client_config) ) }

  fc::optional<name_record> client::lookup_name( const std::string& name )
  { try {
     return my->_chan->lookup_name( name );
  } FC_RETHROW_EXCEPTIONS( warn, "error looking up name '${name}'", ("name",name) ) }

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

  void client::mine_name( const std::string& bitname_id, const fc::ecc::public_key& name_key)
  {
     my->_names_to_mine[bitname_id] = name_key;
     my->start_mining();
     /*
     my->_pending_regs.store( bitname_id, name_key );
     my->_miner.set_name( bitname_id, name_key );
     my->_miner.start( my->_config.max_mining_effort );
     */
  }

  const std::unordered_map<std::string,fc::ecc::public_key_data>&  client::actively_mined_names()const
  {
    return my->_names_to_mine;
  }
  void                                       client::stop_mining_name( const std::string& bitname_id )
  {
     my->_names_to_mine.erase( bitname_id );
     my->start_mining(); // reset mining just incase we are currently mining bitname_id
  }

} } // bts::bitname
