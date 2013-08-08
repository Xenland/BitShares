#include <bts/bitname/name_channel.hpp>
#include <bts/bitname/name_messages.hpp>
#include <bts/bitname/name_db.hpp>
#include <bts/network/server.hpp>
#include <bts/network/channel.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/log/logger.hpp>


namespace bts { namespace bitname {

  using namespace bts::network;
  namespace detail 
  { 
    class name_channel_impl : public bts::network::channel
    {
       public:
          name_channel_impl()
          :del(nullptr){}

          name_channel_delegate*       del;
          bts::peer::peer_channel_ptr  peers;
          network::channel_ptr         chan;
                                    
          name_db                      ndb;

          std::shared_ptr<bts::network::channel> chan_handler;

         void handle_message( const connection_ptr& con, const message& m )
         {
            fc::datastream<const char*>  ds(m.data.data(), m.data.size() );
            fc::unsigned_int msg_type;
            fc::raw::unpack( ds, msg_type );

            if( msg_type == name_inv_msg::type )
            {
               name_inv_msg msg;
               fc::raw::unpack(ds,msg);
               handle_name_inv( con, msg );
            }
            /*
            else if( msg_type == name_trx_msg::type )
            {

            }
            else if( msg_type == block_headers_msg::type )
            {

            }
            else if( msg_type == block_msg::type )
            {

            }
            else if( msg_type == get_headers_msg::type )
            {

            }
            else if( msg_type == get_name_trx_msg::type )
            {
            }
            else if( msg_type == get_block_msg::type )
            {
            }
            */
            else
            {
               wlog( "unexpected message type ${msg_type}", ("msg_type",msg_type) );
            }
         }

         /**
          *  Assuming the difficulty is high enough, fetch it from the remote machine 
          */
         void handle_name_inv( const connection_ptr& con, const name_inv_msg& m )
         {
            
         }

         /**
          *  Assuming the transaction is a valid, unique name or renewal add it to
          *  the working set and broadcast an name_inv message
          */
         void handle_name_trx( const connection_ptr& con, const name_trx_msg& m )
         {

         }
          
         /**
          *  This message is received in response to a get_block request which 
          *  presumably assumes we are downloading a new chain.   Validate that
          *  all transactions have a valid POW and that all registration trx
          *  are valid.  If so, append it to the block chain.
          */
         void handle_name_block( const connection_ptr& con, const block_msg&  m )
         {

         }

         /**
          *  This message presumes a prior inv message was sent to the given host
          */
         void handle_get_name_trx( const connection_ptr& con, const get_trx_msg& m )
         {

         }
         
         /**
          *  Lookup the block in the database and return it to the user.
          */
         void handle_get_block( const connection_ptr& con, const get_block_msg& m )
         {

         }

         void handle_get_headers( const connection_ptr& con, const get_headers_msg& m )
         {

         }

         void handle_block_headers( const connection_ptr& con, const block_headers_msg& m )
         {

         }



    };

  } // namespace detail

  name_channel::name_channel( const bts::peer::peer_channel_ptr& n )
  :my( new detail::name_channel_impl() )
  {
     my->peers = n;
     my->peers->subscribe_to_channel( channel_id(network::name_proto), my );
  }

  name_channel::~name_channel() 
  { 
     my->peers->unsubscribe_from_channel( channel_id(network::name_proto) );
  } 

  void name_channel::configure( const name_channel::config& c )
  {
      my->ndb.open( c.name_db_dir, true/*create*/ );

      // TODO: connect to the network and attempt to download the chain...
      //      *  what if no peers on on the name channel ??  * 
      //         I guess when I do connect to a peer on this channel they will
      //         learn that I am subscribed to this channel... 
  }
  void name_channel::set_delegate( name_channel_delegate* d )
  {
     my->del = d;
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
