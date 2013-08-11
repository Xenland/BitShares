#include <bts/bitname/name_client.hpp>
#include <bts/bitname/name_channel.hpp>
#include <fc/crypto/hex.hpp>

namespace bts { namespace bitname {
  
  namespace detail 
  {
     class client_impl
     {
        public:
          name_channel_ptr chan;
     };

  } // detail

  client::client( const peer::peer_channel_ptr& peer_ch )
  :my( new detail::client_impl() )
  {
      my->chan = std::make_shared<bts::bitname::name_channel>(peer_ch);
  }
  client::~client(){}

  name_record client::lookup_name( const std::string& name )
  {
     try {
       name_header head = my->chan->lookup_name( name );
       name_record rec;
       rec.last_update  = head.utc_sec;
       rec.num_updates  = head.renewal;
       rec.revoked      = !!head.cancel_sig;
       rec.name_hash    = fc::to_hex( (char*)&head.name_hash, sizeof( head.name_hash)  );
       rec.name         = name;
       FC_ASSERT( !!head.key )
       rec.pub_key      = *head.key;
       
       return rec;
     } FC_RETHROW_EXCEPTIONS( warn, "error looking up name '${name}'", ("name",name) );
  }

  name_record client::reverse_name_lookup( const fc::ecc::public_key& k )
  {
    FC_ASSERT( !"Not Implemented" );
  }

  /**
   *  This wrapper on fc:::ecc::public_key is exposed on this client API so that it may
   *  be called via JSON-RPC by 3rd party apps that don't have the crypto methods required
   *  to derive the public key from the digest & signature.
   */
  fc::ecc::public_key client::verify_signature( const fc::sha256& digest, const fc::ecc::compact_signature& sig )
  {
     return fc::ecc::public_key( sig, digest );
  }

  std::string client::register_name( const std::string& name, const fc::ecc::public_key& k )
  {
    FC_ASSERT( !"Not Implemented" );
  }

  fc::ecc::compact_signature client::sign( const fc::sha256& digest, const std::string& name )
  {
    FC_ASSERT( !"Not Implemented" );
  }

} } // bts::bitname
