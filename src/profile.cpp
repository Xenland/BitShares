#include <bts/profile.hpp>
#include <bts/keychain.hpp>
#include <bts/addressbook/addressbook.hpp>

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
            keychain                     _keychain;
            addressbook::addressbook_ptr _addressbook;
            bitchat::message_db_ptr      _message_db;

     };

  } // namespace detail

  profile::profile()
  :my( new detail::profile_impl() )
  {
    my->_addressbook = std::make_shared<addressbook::addressbook>();
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
      auto profile_cfg_key         = fc::sha512::hash( password.c_str(), password.size() );
      auto stretched_seed_data     = fc::aes_load( profile_dir / ".stretched_seed", profile_cfg_key );
     
      my->_keychain.set_seed( fc::raw::unpack<fc::sha512>(stretched_seed_data) );
      my->_addressbook->open( profile_dir / "addressbook" );

  } FC_RETHROW_EXCEPTIONS( warn, "", ("profile_dir",profile_dir) ) }

  std::vector<identity>   profile::identities()const
  {
     std::vector<identity> idents;
     return idents;
  }
  
  void    profile::store_identity( const identity& id )
  {
  }
  
  /**
   *  Checks the transaction to see if any of the inp
   */
  //void  profile::cache( const bts::blockchain::meta_transaction& mtrx );
  void    profile::cache( const bts::bitchat::decrypted_message& msg    )
  {
  }
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
