#pragma once
#include <bts/bitname/name_channel.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/optional.hpp>
#include <fc/io/varint.hpp>

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

  /**
   *  These methods define the bitname JSON-RPC client used for external applications to lookup information 
   *  about names and validate logins.
   *
   *  Website Login Protocol:
   *
   *    Browser makes Request:
   *    https://domain/request_login
   *    {
   *      "user_nonce" : "nonce"
   *    }
   *    
   *    Response: 
   *    {   
   *        "user_nonce"     : USER_PROVIDED, 
   *        "host_nonce"     : host_provided, 
   *        "host_bitname"   : "name...",   note: bitdns may be better for this side 
   *        "host_signature" : "..."
   *    }
   *
   *    Browser then queries local bitname service:
   *      1) server_record  = local_service->lookup( host_bitname )
   *      2) server_pub_key = local_service->verify_signature( sha256( user_nonce | host_nonce | host_ip ), host_signature )
   *      3) assert( server_pub_key == server_record.pub_key )
   *      4) Ask user if they would like to login (and ask which ID if they have multiple)... display the name.
   *      5) If the user says ok... then login with the following request
   *
   *    https://domain/login
   *    { 
   *      "user_bitname" : 
   *      "user_nonce" : 
   *      "host_nonce" : 
   *      "host_signature" : 
   *      "user_signature" :
   *    }
   *
   *    Response:
   *    Session Cookie... 
   *    {
   *       "status" : "ok"
   *       "status" : "denied"
   *    }
   */
  class client 
  {
     public:
       client( const bts::bitname::name_channel_ptr& namechan );
       ~client();

       name_record                    lookup_name( const std::string& name );
       name_record                    reverse_name_lookup( const fc::ecc::public_key& k );
       fc::ecc::public_key            verify_signature( const fc::sha256& digest, const fc::ecc::compact_signature& sig );
       fc::ecc::compact_signature     sign( const fc::sha256& digest, const std::string& name );

       /**
        *  Attempts to register the name to the given public key.
        */
       std::string                    register_name( const std::string& name, const fc::ecc::public_key& );
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

#include <fc/ptr.hpp>
FC_STUB( bts::bitname::client, 
  (lookup_name)
  (reverse_name_lookup)
  (verify_signature)
  (sign)
  (register_name)
  )
