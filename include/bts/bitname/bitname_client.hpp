#pragma once
#include <bts/peer/peer_channel.hpp>
#include <bts/bitname/bitname_block.hpp>
#include <bts/bitname/bitname_record.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>
#include <fc/io/varint.hpp>
#include <fc/filesystem.hpp>

#include <unordered_map>


namespace bts { namespace bitname {

  namespace detail { class client_impl; }

  class client_delegate 
  {
     public:
        virtual ~client_delegate(){}
        /**
         *  Called when a new valid name trx is found 
         *  (either by mining or reported form the network).  
         */
        void bitname_header_pending( const bitname::name_header& h ){};  

        /**
         *  Called anytime the head block is extended or replaced.
         */
        void bitname_block_added( const bitname::name_block& h ){};  
  };

  /**
   *  These methods define the bitname JSON-RPC client used for external applications to lookup information 
   *  about names and validate logins.
   *
   */
  class client 
  {
     public:
       client( const bts::peer::peer_channel_ptr& peer_chan );
       ~client();

       struct config
       {
          config()
          :max_mining_effort(0.25){} // TODO: remove magic number... 

          fc::path data_dir;
          double   max_mining_effort;
       };

       void set_delegate( client_delegate* client_del );
       void configure( const config& client_config );

       fc::optional<name_record>      lookup_name( const std::string& name );
       name_record                    reverse_name_lookup( const fc::ecc::public_key& k );
       fc::ecc::public_key            verify_signature( const fc::sha256& digest, const fc::ecc::compact_signature& sig );

       fc::time_point                 get_current_chain_time()const;
       uint16_t                       get_current_difficulty()const;

       /**
        *  Attempts to register the name to the given public key, throws an exception if the name is already
        *  registered on the network.
        */
       void  mine_name( const std::string& bitname_id, const fc::ecc::public_key& );
       const std::unordered_map<std::string,fc::ecc::public_key_data>&  actively_mined_names()const;
       void  stop_mining_name( const std::string& bitname_id );
     private:
       std::unique_ptr<detail::client_impl> my;
  };

  typedef std::shared_ptr<client> client_ptr;

} } // bts::bitname

FC_REFLECT( bts::bitname::client::config,
    (data_dir)
    (max_mining_effort)
    )

