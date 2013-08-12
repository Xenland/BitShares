#include <bts/bitname/bitname_channel.hpp>
#include <bts/bitname/bitname_messages.hpp>
#include <bts/bitname/bitname_db.hpp>
#include <bts/network/server.hpp>
#include <bts/network/channel.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>


namespace bts { namespace bitname {

  using namespace bts::network;
  namespace detail 
  { 
    class chan_data : public network::channel_data
    {
      public:
        std::unordered_set<mini_pow> known_inv;
    };

    class name_channel_impl : public bts::network::channel
    {
       public:
          name_channel_impl()
          :_del(nullptr){}

          name_channel_delegate*       _del;
          bts::peer::peer_channel_ptr  _peers;
          network::channel_id          _chan_id;
                                    
          name_db                      _ndb;

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
          }

          void handle_name_inv( const connection_ptr& con,  chan_data& cdat, const name_inv_message& msg )
          {
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
          }
   
          void handle_name( const connection_ptr& con,  chan_data& cdat, const name_message& msg )
          {
          }
   
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
  }

  name_channel::~name_channel() 
  { 
     my->_peers->unsubscribe_from_channel( my->_chan_id );
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

  void name_channel::submit_name( const name_trx& t )
  {
  }

  void name_channel::submit_block( const name_block& b )
  {
  }

  /**
   *  Performs a lookup in the internal database 
   */
  name_header name_channel::lookup_name( const std::string& name )
  {
    FC_ASSERT( !"Not Implemented" );
  }

} } // bts::bitname
