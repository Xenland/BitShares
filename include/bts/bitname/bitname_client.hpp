#pragma once
#include <bts/peer/peer_channel.hpp>
#include <bts/bitname/bitname_block.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>
#include <fc/io/varint.hpp>
#include <fc/filesystem.hpp>

#include <map>


namespace bts { namespace bitname {

  namespace detail { class client_impl; }

  struct name_record
  {
     name_record()
     :revoked(false){}

     fc::time_point_sec          last_update; ///< the most recent update of this name
     fc::unsigned_int            num_updates; ///< the number of times the name has been updated/renewed
     fc::ecc::public_key         pub_key;     ///< the public key paired to this name.
     bool                        revoked;     ///< this name has been canceled, by its owner (invalidating the public key)
     std::string                 name_hash;   ///< the unique hash assigned to this name, in hex format
     fc::optional<std::string>   name;        ///< if we know the name that generated the hash
  };

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
          :max_mining_effort(0.25){}

          fc::path data_dir;
          double   max_mining_effort;
       };

       void set_delegate( client_delegate* client_del );
       void configure( const config& client_config );

       name_record                    lookup_name( const std::string& name );
       name_record                    reverse_name_lookup( const fc::ecc::public_key& k );
       fc::ecc::public_key            verify_signature( const fc::sha256& digest, const fc::ecc::compact_signature& sig );
       fc::ecc::compact_signature     sign( const fc::sha256& digest, const std::string& name );

       fc::time_point                 get_current_chain_time()const;
       uint16_t                       get_current_difficulty()const;

       /**
        *  Attempts to register the name to the given public key, throws an exception if the name is already
        *  registered on the network.
        */
       void                                       register_name( const std::string& bitname_id, const fc::ecc::public_key& );
       std::map<std::string,fc::ecc::public_key>  pending_name_registrations()const;
       void                                       cancel_name_registration( const std::string& bitname_id );
     private:
       std::unique_ptr<detail::client_impl> my;
  };

  typedef std::shared_ptr<client> client_ptr;

} } // bts::bitname

FC_REFLECT( bts::bitname::name_record,
    (last_update)
    (num_updates)
    (pub_key)
    (revoked)
    (name_hash)
    (name)
  )
FC_REFLECT( bts::bitname::client::config,
    (data_dir)
    (max_mining_effort)
    )

#include <fc/ptr.hpp>
FC_STUB( bts::bitname::client, 
  (lookup_name)
  (reverse_name_lookup)
  (verify_signature)
  (sign)
  (register_name)
  )
