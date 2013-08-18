#include <bts/bitname/bitname_channel.hpp>
#include <bts/bitname/bitname_messages.hpp>
#include <bts/bitname/bitname_db.hpp>
#include <bts/network/server.hpp>
#include <bts/network/channel.hpp>
#include <bts/network/broadcast_manager.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/thread/thread.hpp>
#include <fc/log/logger.hpp>

#include <unordered_map>


namespace bts { namespace bitname {

  using namespace bts::network;
  namespace detail 
  { 
    class chan_data : public network::channel_data
    {
      public:
        broadcast_manager<name_hash_type,name_trx>::channel_data         trxs_mgr;
        broadcast_manager<name_id_type,name_block_index>::channel_data   block_mgr;
    };

    class name_channel_impl : public bts::network::channel
    {
       public:
          name_channel_impl()
          :_del(nullptr){}

          name_channel_delegate*                            _del;
          bts::peer::peer_channel_ptr                       _peers;
          network::channel_id                               _chan_id;
                                                            
          name_db                                           _name_db;
          fc::future<void>                                  _fetch_loop;
                                                            
          broadcast_manager<name_hash_type,name_trx>        _trx_broadcast_mgr;
          broadcast_manager<name_id_type,name_block_index>  _block_index_broadcast_mgr;

          void fetch_loop()
          {
             try 
             {
                while( !_fetch_loop.canceled() )
                {
                   broadcast_inv();

                   uint64_t trx_query = 0;
                   if( _trx_broadcast_mgr.find_next_query( trx_query ) )
                   {
                      auto cons = _peers->get_connections( _chan_id );
                      fetch_name_from_best_connection( cons, trx_query );
                      _trx_broadcast_mgr.item_queried( trx_query );
                   }
                   
                   /* By using a random sleep we give other peers the oppotunity to find
                    * out about messages before we pick who to fetch from.
                    * TODO: move constants to config.hpp
                    *
                    * TODO: fetch set your fetch order based upon how many times we have received
                    *        an inv regarding a particular item.
                    */
                   fc::usleep( fc::microseconds( (rand() % 20000) + 100) ); // note: usleep(0) sleeps forever... perhaps a bug?
                }
             } 
             catch ( const fc::exception& e )
             {
               wlog( "${e}", ("e", e.to_detail_string()) );
             }
          }

          /**
           *  Send any new inventory items that we have received since the last
           *  broadcast to all connections that do not know about the inv item.
           */
          void broadcast_inv()
          { try {
              if( _trx_broadcast_mgr.has_new_since_broadcast() || _block_index_broadcast_mgr.has_new_since_broadcast() )
              {
                 auto cons = _peers->get_connections( _chan_id );
                 if( _trx_broadcast_mgr.has_new_since_broadcast() )
                 {
                   for( auto c = cons.begin(); c != cons.end(); ++c )
                   {
                     name_inv_message inv_msg;
                 
                     chan_data& con_data = get_channel_data( *c );
                     inv_msg.names = _trx_broadcast_mgr.get_inventory( con_data.trxs_mgr.known_keys() );
                 
                     if( inv_msg.names.size() )
                     {
                       (*c)->send( network::message(inv_msg,_chan_id) );
                     }
                     con_data.trxs_mgr.update_known( inv_msg.names );
                   }
                   _trx_broadcast_mgr.set_new_since_broadcast(false);
                 }
                 
                 if( _block_index_broadcast_mgr.has_new_since_broadcast() )
                 {
                   for( auto c = cons.begin(); c != cons.end(); ++c )
                   {
                     block_inv_message inv_msg;
                 
                     chan_data& con_data = get_channel_data( *c );
                     inv_msg.block_ids = _block_index_broadcast_mgr.get_inventory( con_data.block_mgr.known_keys() );
                 
                     if( inv_msg.block_ids.size() )
                     {
                       (*c)->send( network::message(inv_msg,_chan_id) );
                     }
                     con_data.block_mgr.update_known( inv_msg.block_ids );
                   }
                   _block_index_broadcast_mgr.set_new_since_broadcast(false);
                 }
             }
          } FC_RETHROW_EXCEPTIONS( warn, "error broadcasting bitname inventory") } // broadcast_inv


          /**
           *   For any given message id, there are many potential hosts from which it could be fetched.  We
           *   want to distribute the load across all hosts equally and therefore, the best one to fetch from
           *   is the host that we have fetched the least from and that has fetched the most from us.
           */
          void fetch_name_from_best_connection( const std::vector<connection_ptr>& cons, uint64_t id )
          { try {
             // if request is made, move id from unknown_names to requested_msgs 
             // TODO: update this algorithm to be something better. 
             for( uint32_t i = 0; i < cons.size(); ++i )
             {
                 chan_data& chan_data = get_channel_data(cons[i]); 
                 if( !chan_data.trxs_mgr.knows( id ) && !chan_data.trxs_mgr.has_pending_request() )
                 {
                    chan_data.trxs_mgr.requested(id);
                    cons[i]->send( network::message( get_name_message( id ), _chan_id ) );
                    return;
                 }
             }
          } FC_RETHROW_EXCEPTIONS( warn, "error fetching name ${name_hash}", ("name_hash",id) ) }


          /**
           *  Get or create the bitchat channel data for this connection and return
           *  a reference to the result.
           */
          chan_data& get_channel_data( const connection_ptr& c )
          {
              auto cd = c->get_channel_data( _chan_id );
              if( !cd )
              {
                 cd = std::make_shared<chan_data>();
                 c->set_channel_data( _chan_id, cd );
              }
              chan_data& cdat = cd->as<chan_data>();
              return cdat;
          }


          void handle_message( const connection_ptr& con, const message& m )
          {
             chan_data& cdat = get_channel_data(con);
   
             ilog( "${msg_type}", ("msg_type", (bitname::message_type)m.msg_type ) );
             
             switch( (bitname::message_type)m.msg_type )
             {
                 case name_inv_msg:
                   handle_name_inv( con, cdat, m.as<name_inv_message>() );
                   break;
                 case block_inv_msg:
                   handle_block_inv( con, cdat, m.as<block_inv_message>() );
                   break;
                 case get_name_inv_msg:
                   handle_get_name_inv( con, cdat, m.as<get_name_inv_message>() );
                   break;
                 case get_headers_msg:
                   handle_get_headers( con, cdat, m.as<get_headers_message>() );
                   break;
                 case get_block_msg:
                   handle_get_block( con, cdat, m.as<get_block_message>() );
                   break;
                 case get_name_msg:
                   handle_get_name( con, cdat, m.as<get_name_message>() );
                   break;
                 case name_msg:
                   handle_name( con, cdat, m.as<name_message>() );
                   break;
                 case block_msg:
                   handle_block( con, cdat, m.as<block_message>() );
                   break;
                 case headers_msg:
                   handle_headers( con, cdat, m.as<headers_message>() );
                   break;
                 default:
                   wlog( "unknown bitname message type ${msg_type}", ("msg_type", m.msg_type ) );
             }
          } // handle_message

          void handle_name_inv( const connection_ptr& con,  chan_data& cdat, const name_inv_message& msg )
          {
              ilog( "inv: ${msg}", ("msg",msg) );
              for( auto itr = msg.names.begin(); itr != msg.names.end(); ++itr )
              {
                 _trx_broadcast_mgr.received_inventory_notice( *itr ); 
              }
              cdat.trxs_mgr.update_known( msg.names );
          }
   
          void handle_block_inv( const connection_ptr& con,  chan_data& cdat, const block_inv_message& msg )
          {
              ilog( "inv: ${msg}", ("msg",msg) );
              for( auto itr = msg.block_ids.begin(); itr != msg.block_ids.end(); ++itr )
              {
                 _block_index_broadcast_mgr.received_inventory_notice( *itr );
              }
              cdat.block_mgr.update_known( msg.block_ids );
          }
   
          void handle_get_name_inv( const connection_ptr& con,  chan_data& cdat, const get_name_inv_message& msg )
          {
              name_inv_message reply;
              reply.names = _trx_broadcast_mgr.get_inventory( cdat.trxs_mgr.known_keys() );
              cdat.trxs_mgr.update_known( reply.names );
              con->send( network::message(reply,_chan_id) );
          }
   
          void handle_get_headers( const connection_ptr& con,  chan_data& cdat, const get_headers_message& msg )
          {
         //     _name_db->get_header_ids
          }
   

          void handle_get_block( const connection_ptr& con,  chan_data& cdat, const get_block_message& msg )
          {
              // TODO: charge POW for this...
              auto block = _name_db.fetch_block( msg.block_id );
              con->send( network::message( block_message( std::move(block) ), _chan_id ) );
          }
   
          void handle_get_name( const connection_ptr& con,  chan_data& cdat, const get_name_message& msg )
          {
             ilog( "${msg}", ("msg",msg) );
             const fc::optional<name_trx>& trx = _trx_broadcast_mgr.get_value( msg.name_hash );
             if( !trx ) // must be a db
             {
                auto trx = _name_db.fetch_trx( msg.name_hash );
                con->send( network::message( name_message( trx ), _chan_id ) );
             }
             else
             {
                con->send( network::message( name_message( *trx ), _chan_id ) );
             }
          }
   
          void handle_name( const connection_ptr& con,  chan_data& cdat, const name_message& msg )
          { try {
             ilog( "${msg}", ("msg",msg) );
             cdat.trxs_mgr.received_response( msg.name.name_hash );
             try { 
                _name_db.validate_trx( msg.name ); 
                _trx_broadcast_mgr.validated( msg.name.name_hash, msg.name, true );
             } 
             catch ( const fc::exception& e )
             {
                // TODO: connection just sent us an invalid trx... what do we do...
                wlog( "${e}", ("e",e.to_detail_string()) ); 
                _trx_broadcast_mgr.validated( msg.name.name_hash, msg.name, false );
                throw;
             }
          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg", msg) ) }
   
          void handle_block( const connection_ptr& con,  chan_data& cdat, const block_message& msg )
          { try {
               // TODO: make sure that I requrested this block... 
               _name_db.push_block( msg.block ); 
          } FC_RETHROW_EXCEPTIONS( warn,"handling block ${block}", ("block",msg) ) }
   
          void handle_headers( const connection_ptr& con,  chan_data& cdat, const headers_message& msg )
          {

          }
    };

  } // namespace detail

  name_channel::name_channel( const bts::peer::peer_channel_ptr& n )
  :my( new detail::name_channel_impl() )
  {
     my->_peers = n;
     my->_chan_id = channel_id(network::name_proto,0);
     my->_peers->subscribe_to_channel( my->_chan_id, my );
     my->_fetch_loop = fc::async( [=](){ my->fetch_loop(); } );
  }

  name_channel::~name_channel() 
  { 
     my->_peers->unsubscribe_from_channel( my->_chan_id );
     my->_del = nullptr;
     try {
        my->_fetch_loop.cancel();
        my->_fetch_loop.wait();
     } 
     catch ( ... ) 
     {
        wlog( "unexpected exception ${e}", ("e", fc::except_str()));
     }
  } 

  void name_channel::configure( const name_channel::config& c )
  {
      my->_name_db.open( c.name_db_dir, true/*create*/ );

      // TODO: connect to the network and attempt to download the chain...
      //      *  what if no peers on on the name channel ??  * 
      //         I guess when I do connect to a peer on this channel they will
      //         learn that I am subscribed to this channel... 
  }
  void name_channel::set_delegate( name_channel_delegate* d )
  {
     my->_del = d;
  }

  void name_channel::submit_name( const name_trx& new_name_trx )
  { try {
     //FC_ASSERT( fc::time_point::now() - new_name_trx.utc_sec  <  fc::seconds(60*10) ); // TODO: remove hardcode time window
     //TODO  FC_ASSERT( new_name_trx.utc_sec <= fc::time_point_sec(fc::time_point::now()) );

     // TODO: verify new_name_trx.prev == current head... (or head.prev and id() < head )

     //my->_pending_names[new_name_trx.name_hash] = new_name_trx;
     //my->_new_names.push_back( new_name_trx.name_hash );
     my->_name_db.validate_trx( new_name_trx );
     my->_trx_broadcast_mgr.validated( new_name_trx.name_hash, new_name_trx, true );
  } FC_RETHROW_EXCEPTIONS( warn, "error submitting name", ("new_name_trx", new_name_trx) ) }

  void name_channel::submit_block( const name_block& b )
  {
     // TODO: verify that we have all trx in merkle tree... 
     // block should be in our DB... 
     // ... 
  }

  /**
   *  Performs a lookup in the internal database 
   */
  name_header name_channel::lookup_name( const std::string& name )
  {
    FC_ASSERT( !"Not Implemented" );
  }

} } // bts::bitname
