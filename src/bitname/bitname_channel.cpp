#include <bts/bitname/bitname_channel.hpp>
#include <bts/bitname/bitname_messages.hpp>
#include <bts/bitname/bitname_db.hpp>
#include <bts/network/server.hpp>
#include <bts/network/channel.hpp>
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
        std::unordered_set<mini_pow> known_block_inv;
        std::unordered_set<uint64_t> known_name_inv;
    };

    class name_channel_impl : public bts::network::channel
    {
       public:
          name_channel_impl()
          :_del(nullptr){}

          name_channel_delegate*                       _del;
          bts::peer::peer_channel_ptr                  _peers;
          network::channel_id                          _chan_id;
                                                       
          name_db                                      _ndb;
          fc::future<void>                             _fetch_loop;

          /// messages received since last inv broadcast
          std::vector<uint64_t>                        _new_names; 

          /// names not yet in any block, available for request
          std::unordered_map<uint64_t, name_trx>       _pending_names;

          /// new name updates that have come in
          std::unordered_set<uint64_t>                 _unknown_names;
          std::unordered_map<uint64_t,fc::time_point>  _requested_names; // messages that we have requested but not yet received


          void fetch_loop()
          {
             try {
                while( !_fetch_loop.canceled() )
                {
                   broadcast_inv();
                   if( _unknown_names.size()  )
                   {
                      auto cons = _peers->get_connections( _chan_id );
                      // copy so we don't hold shared state in the iterators
                      // while we iterate and send fetch requests
                      auto tmp = _unknown_names; 
                      for( auto itr = tmp.begin(); itr != tmp.end(); ++itr )
                      {
                          fetch_name_from_best_connection( cons, *itr );
                      }
                   }
                   /* By using a random sleep we give other peers the oppotunity to find
                    * out about messages before we pick who to fetch from.
                    * TODO: move constants to config.hpp
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
              if( _new_names.size() )
              {
                auto cons = _peers->get_connections( _chan_id );
                for( auto c = cons.begin(); c != cons.end(); ++c )
                {
                  name_inv_message msg;

                  chan_data& cd = get_channel_data( *c );
                  for( uint32_t i = 0; i < _new_names.size(); ++i )
                  {
                     if( cd.known_name_inv.insert( _new_names[i] ).second )
                     {
                        msg.names.push_back( _new_names[i] );
                     }
                  }

                  if( msg.names.size() )
                  {
                    (*c)->send( network::message(msg,_chan_id) );
                  }
                }
                _new_names.clear();
              }
              // TODO: broadcast new blocks...
          } FC_RETHROW_EXCEPTIONS( warn, "error broadcasting bitname inventory") } // broadcast_inv


          /**
           *   For any given message id, there are many potential hosts from which it could be fetched.  We
           *   want to distribute the load across all hosts equally and therefore, the best one to fetch from
           *   is the host that we have fetched the least from and that has fetched the most from us.
           */
          void fetch_name_from_best_connection( const std::vector<connection_ptr>& cons, const uint64_t& id )
          { try {
             // if request is made, move id from unknown_names to requested_msgs 
             // TODO: update this algorithm to be something better. 
             for( uint32_t i = 0; i < cons.size(); ++i )
             {
                 chan_data& cd = get_channel_data(cons[i]); 
                 if( cd.known_name_inv.find( id ) !=  cd.known_name_inv.end() )
                 {
                    _requested_names[id] = fc::time_point::now();
                    _unknown_names.erase(id);
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
                 case get_block_inv_msg:
                   handle_get_block_inv( con, cdat, m.as<get_block_inv_message>() );
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
                 cdat.known_name_inv.insert( *itr );
                 if( _pending_names.find( *itr ) == _pending_names.end() )
                 {
                    _unknown_names.insert( *itr );
                 }
              }
          }
   
          void handle_block_inv( const connection_ptr& con,  chan_data& cdat, const block_inv_message& msg )
          {
          }
   
          void handle_get_name_inv( const connection_ptr& con,  chan_data& cdat, const get_name_inv_message& msg )
          {
          }
   
          void handle_get_block_inv( const connection_ptr& con,  chan_data& cdat, const get_block_inv_message& msg )
          {
          }
   
          void handle_get_headers( const connection_ptr& con,  chan_data& cdat, const get_headers_message& msg )
          {
          }
   
          void handle_get_block( const connection_ptr& con,  chan_data& cdat, const get_block_message& msg )
          {
          }
   
          void handle_get_name( const connection_ptr& con,  chan_data& cdat, const get_name_message& msg )
          {
             ilog( "${msg}", ("msg",msg) );
             auto pend_name_itr = _pending_names.find( msg.name_hash );
             if( pend_name_itr == _pending_names.end() )
             {
                // must be a DB lookup... 
                wlog( "TODO: perform db lookup" );
             }
             else
             {
                con->send( network::message( name_message( pend_name_itr->second ), _chan_id ) );
             }
          }
   
          void handle_name( const connection_ptr& con,  chan_data& cdat, const name_message& msg )
          { try {
             ilog( "${msg}", ("msg",msg) );
             // TODO: verify that we requested this msg.
             // validate that the contained name is valid based upon the current trxdb state
             // if it is, add it to the pending name_trx queue..
             if( _pending_names.find(msg.name.name_hash) == _pending_names.end() )
             {
                _pending_names[msg.name.name_hash] = msg.name;
                _new_names.push_back( msg.name.name_hash );
                FC_ASSERT( _del != nullptr );
                _del->pending_name_registration( msg.name );
             }
             else
             {
                wlog( "duplicate name trx received ${name_trx}", ("name_trx",msg) );
             }
          } FC_RETHROW_EXCEPTIONS( warn, "", ("msg", msg) ) }
   
          void handle_block( const connection_ptr& con,  chan_data& cdat, const block_message& msg )
          {
          }
   
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
      my->_ndb.open( c.name_db_dir, true/*create*/ );

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
  {
     FC_ASSERT( fc::time_point::now() - new_name_trx.utc_sec  <  fc::seconds(60*10) ); // TODO: remove hardcode time window
   //TODO  FC_ASSERT( new_name_trx.utc_sec <= fc::time_point_sec(fc::time_point::now()) );

     // TODO: verify new_name_trx.prev == current head... (or head.prev and id() < head )

     my->_pending_names[new_name_trx.name_hash] = new_name_trx;
     my->_new_names.push_back( new_name_trx.name_hash );
  }

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
