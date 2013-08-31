#pragma once
#include <fc/fileystem.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/time.hpp>

namespace bts { namespace bitchat {

  namespace detail { class message_db_impl; }


  struct message_header
  {
      fc::enum_type<uint32_t,private_message_type>   type;
      fc::enum_type<uint32_t,encryption_type>        encryption_type;
      fc::enum_type<uint32_t,compression_type>       compression_type;
      fc::time_point_sec                             received_time;
      fc::ecc::public_key_data                       to_key;
      fc::ecc::public_key_data                       from_key;
      fc::uint256                                    digest;
      fc::ecc::compact_signature                     from_sig;
      fc::time_point_sec                             from_sig_time;
  };
  
  /**
   *  Stores and indexes messages that are known to this
   *  client.  Messages are stored in their encrypted form
   *  but the index is not encrypted.  
   *
   *  @note the message DB is something that will probably
   *  change frequently and we will want to support upgrade
   *  migrations.
   */
  class message_db
  {
     public:
       message_db();
       ~message_db();
       
       /**
        *  @param key - defines the AES key used to encrypt/decrypt the messages stored
        *               in the database.
        *  @throw if the key is invalid or the database is unable to be created.
        */
       void open( const fc::path& dbdir, const fc::uint512_t& key, bool create = true );
       
       void store( const decrypted_message& m );
       
       std::vector<message_header>  fetch_headers( private_message_type t, 
                                                   fc::time_point_sec from_time, 
                                                   fc::time_point_sec to_time,
                                                   fc::optional<fc::ecc::public_key_data> to_key,
                                                   fc::optional<fc::ecc::public_key_data> from_key );
       
       std::vector<char>            fetch_data(  const fc::uint256& digest );
     private:
       std::unique_ptr<detail::message_db_impl> my;
  };

  typedef std::make_shared<message_db> message_db_ptr;

} } // bts::bitchat

FC_REFLECT( bts::bitchat::message_header, 
    (type)
    (encryption_type)
    (compression_type)
    (received_time)
    (to_key)
    (digest) 
    (from_key)
    (from_sig)
    (from_sig_time)
    )
