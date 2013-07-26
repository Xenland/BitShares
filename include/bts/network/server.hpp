#pragma once
#include <bts/network/message.hpp>
#include <bts/network/channel.hpp>
#include <bts/network/stcp_socket.hpp>
#include <bts/db/fwd.hpp>
#include <bts/config.hpp>

#include <set>

namespace bts { namespace network {
  namespace detail { class server_impl; };

  class connection;
  typedef std::shared_ptr<connection> connection_ptr;
  
  /**
   * @brief defines the set of callbacks that a server provides.
   *
   */
  class server_delegate
  {
     public:
       virtual ~server_delegate(){}

       virtual void on_connected( const connection_ptr& c ){}
       virtual void on_disconnected( const connection_ptr& c ){}
  };


  /**
   *   Abstracts the process of sending and receiving messages 
   *   on the network.  All messages are broadcast or received
   *   on a particular channel and each channel defines a protocol
   *   of message types supported on that channel.
   *
   *   The server will organize connections into a KAD tree for
   *   each subscribed channel.  The ID of a node will be the
   *   64 bit truncated SHA256(IP:PORT) which should distribute 
   *   peers well and facilitate certain types of protocols which
   *   do not want to broadcast everywhere, but instead to perform
   *   targeted lookup of data in a hash table.
   */
  class server
  {
    public:
        struct config
        {
            config()
            :port(DEFAULT_SERVER_PORT){}
            uint16_t                 port;  ///< the port to listen for incoming connections on.
            std::string              chain; ///< the name of the chain this server is operating on (test,main,etc)

            std::vector<std::string> bootstrap_endpoints; // host:port strings for initial connection to the network.

            std::vector<std::string> blacklist;  // host's that are blocked from connecting
        };
        
        server();
        ~server();

        void close();

        /**
         *  broadcasts the new channel subscription to all connected nodes. If less than
         *  the minimum number of connections exist to this channel, new connections are
         *  opened.
         */
        void subscribe_to_channel( const channel_id& chan, const channel_ptr& c );
        void unsubscribe_from_channel( const channel_id& chan );

        /**
         *  @note del must out live this server and the server does not
         *        take ownership of the delegate.
         */
        void set_delegate( server_delegate* del );
        
        /**
         * Attempts to open at least count connections to 
         * peers.  Returns a valid connection_ptr on success after calling
         * the server_delegate::on_connected() method.
         *
         * @throw if there is an error connecting to EP
         */
        connection_ptr connect_to( const fc::ip::endpoint& ep );

        /**
         *  Sets up the ports and performs other one-time
         *  initialization.   
         *
         *  TODO: can the user change settings while things are
         *  running?
         */
        void configure( const config& c );


        /**
         *  @return a set of connections that are subscribed to a particular channel, prioritized
         *          by their XOR distance from the target.
         */
        std::vector<connection_ptr> connections_for_channel( const channel_id& c,
                                                             const fc::sha1& near = fc::sha1() );

        /**
         * Get all connections for any channel.
         */
        std::vector<connection_ptr> get_connections()const;

        /** send the message to all connected peers */
        void broadcast( const message& m );
      private:
        std::unique_ptr<detail::server_impl> my;
  };

  typedef std::shared_ptr<server> server_ptr;

} } // bts::server

FC_REFLECT( bts::network::server::config, (port)(chain)(bootstrap_endpoints)(blacklist) )
