#pragma once
#include <bts/bitname/name_block.hpp>
#include <bts/peer/peer_channel.hpp>
#include <bts/network/server.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace bitname {

  namespace detail { class name_channel_impl; }

  class name_channel_delegate
  {
    public:
       virtual ~name_channel_delegate(){}

       /**
        *   Called any time a new & valid name reg trx is received
        */
       virtual void pending_name_registration( const name_trx&  ) = 0;

       /**
        *   Called any time a new & valid name block is added to the
        *   chain or replaces the head block.
        */
       virtual void name_block_added( const name_block& b ) = 0;
  };


  /**
   *  Subscribes to the name registration channel, downloads the
   *  database, and provides callbacks anytime something changes.
   */
  class name_channel 
  {
      public:
        struct config
        {
           fc::path name_db_dir;
        };

        name_channel( const bts::peer::peer_channel_ptr& n );
        ~name_channel();

        void configure( const name_channel::config& c );
        void set_delegate( name_channel_delegate* d );

        void submit_name( const name_trx& t );
        void submit_block( const name_block& b );

        /**
         *  Performs a lookup in the internal database and throws
         *  an exception if the name is not found.
         */
        name_header lookup_name( const std::string& name );

      private:
        std::shared_ptr<detail::name_channel_impl> my;
  };



 } } // bts::bitname
