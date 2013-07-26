#pragma once
#include <bts/network/stcp_socket.hpp>
#include <bts/network/message.hpp>
#include <bts/mini_pow.hpp>
#include <fc/exception/exception.hpp>

namespace bts { namespace network {
  
   namespace detail { class connection_impl; }

   class connection;
   class message;
   typedef std::shared_ptr<connection> connection_ptr;

   /** 
    * @brief defines callback interface for connections
    */
   class connection_delegate
   {
      public:
        virtual ~connection_delegate(){}; 
        virtual void on_connection_message( connection& c, const message& m ){};
        virtual void on_connection_disconnected( connection& c ){}
   };

   /**
    * Common base class for channel data associated with a specific connection.
    *
    * Many channels need to maintain per-connection data to track broadcast
    * state and potential abuses.  The connection object maintains a mapping from
    * channel id to channel data.  Each channel that needs custom data stored with
    * the connection should derive a data class from channel_data and then
    * use dynamic_cast to recover the data.
    */
   class channel_data : public std::enable_shared_from_this<channel_data>
   {
      public:
          virtual ~channel_data(){}

          template<typename T>
          T& as()
          {
              T* p = dynamic_cast<T*>(this);
              FC_ASSERT( p != nullptr );
              return *p;
          }
   };
   typedef std::shared_ptr<channel_data> channel_data_ptr;


   /**
    *  Manages a connection to a remote p2p node. A connection
    *  processes a stream of messages that have a common header 
    *  and ensures everything is properly encrypted.
    *
    *  A connection also allows arbitrary data to be attached to it
    *  for use by other protocols built at higher levels.
    */
   class connection : public std::enable_shared_from_this<connection>
   {
      public:
        connection( const stcp_socket_ptr& c, connection_delegate* d);
        connection( connection_delegate* d );
        ~connection();
   
        stcp_socket_ptr  get_socket()const;
        fc::ip::endpoint remote_endpoint()const;
        
        /**
         *  @return nullptr if no data has been assigned to c
         */
        channel_data_ptr get_channel_data( const channel_id& c )const;

        /**
         *  Sets data associated with c and replaces any existing data.
         */
        void             set_channel_data( const channel_id& c, const channel_data_ptr& d );
   
        void send( const message& m );
   
        void connect( const std::string& host_port );  
        void close();

      private:
        std::unique_ptr<detail::connection_impl> my;
   };

    
} } // bts::network 
