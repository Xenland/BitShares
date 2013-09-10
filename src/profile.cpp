#include <bts/profile.hpp>
#include <bts/keychain.hpp>
#include <bts/addressbook/addressbook.hpp>
#include <bts/db/level_map.hpp>
#include <fc/io/raw.hpp>
#include <fc/io/raw_variant.hpp>

#include <fc/crypto/aes.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/fstream.hpp>

namespace bts {

  namespace detail 
  {
     class profile_impl
     {
        public:
            keychain                             _keychain;
            addressbook::addressbook_ptr         _addressbook;
            bitchat::message_db_ptr              _message_db;
            db::level_map<std::string, identity> _idents;
     };

  } // namespace detail

  profile::profile()
  :my( new detail::profile_impl() )
  {
    my->_addressbook = std::make_shared<addressbook::addressbook>();
    my->_message_db  = std::make_shared<bitchat::message_db>();
  }
  

  profile::~profile()
  {}

  void profile::create( const fc::path& profile_dir, const profile_config& cfg, const std::string& password )
  { try {
       fc::sha512::encoder encoder;
       fc::raw::pack( encoder, password );
       fc::raw::pack( encoder, cfg );
       auto seed             = encoder.result();

       /// note: this could take a minute
       auto stretched_seed   = keychain::stretch_seed( seed );
       
       FC_ASSERT( !fc::exists( profile_dir ) );
       fc::create_directories( profile_dir );
       
       auto profile_cfg_key  = fc::sha512::hash( password.c_str(), password.size() );
       fc::aes_save( profile_dir / ".stretched_seed", profile_cfg_key, fc::raw::pack(stretched_seed) );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("profile_dir",profile_dir)("config",cfg) ) }

  void profile::open( const fc::path& profile_dir, const std::string& password )
  { try {
      fc::create_directories( profile_dir );
      fc::create_directories( profile_dir / "addressbook" );
      fc::create_directories( profile_dir / "idents" );
      fc::create_directories( profile_dir / "messages" );

      auto profile_cfg_key         = fc::sha512::hash( password.c_str(), password.size() );
      auto stretched_seed_data     = fc::aes_load( profile_dir / ".stretched_seed", profile_cfg_key );
     
      my->_keychain.set_seed( fc::raw::unpack<fc::sha512>(stretched_seed_data) );
      my->_addressbook->open( profile_dir / "addressbook" );
      my->_idents.open( profile_dir / "idents" );
      my->_message_db->open( profile_dir / "messages", profile_cfg_key );

  } FC_RETHROW_EXCEPTIONS( warn, "", ("profile_dir",profile_dir) ) }

  std::vector<identity>   profile::identities()const
  { try {
     std::vector<identity> idents;
     for( auto itr = my->_idents.begin(); itr.valid(); ++itr )
     {
       idents.push_back(itr.value());
     }
     return idents;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }
  
  void    profile::store_identity( const identity& id )
  { try {
      my->_idents.store( id.bit_id, id ); 
  } FC_RETHROW_EXCEPTIONS( warn, "", ("id",id) ) }
  
  /**
   *  Checks the transaction to see if any of the inp
   */
  //void  profile::cache( const bts::blockchain::meta_transaction& mtrx );
  void    profile::cache( const bts::bitchat::decrypted_message& msg    )
  { try {
    my->_message_db->store( msg );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",msg)) }
  /*
  std::vector<meta_transaction> profile::get_transactions()const
  {
  }
  */

  bitchat::message_db_ptr       profile::get_inbox()const
  {
    return my->_message_db;
  }

  addressbook::addressbook_ptr  profile::get_addressbook()const
  {
    return my->_addressbook;
  }

  keychain                      profile::get_keychain()const
  {
     return my->_keychain;
  }
  

} // namespace bts
