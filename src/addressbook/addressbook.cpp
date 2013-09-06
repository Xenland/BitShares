#include <bts/addressbook/addressbook.hpp>
#include <bts/db/level_map.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace addressbook {

  namespace detail 
  { 
     class addressbook_impl
     {
        public:
           db::level_map<std::string,contact>       _contact_db;
           db::level_map<std::string,std::string>   _address_index;
     };
  }

  addressbook::addressbook()
  :my( new detail::addressbook_impl() )
  {
  }

  addressbook::~addressbook()
  {
  }

  void addressbook::open( const fc::path& abook_dir )
  { try {

     if( !fc::exists( abook_dir ) )
     {
        fc::create_directories( abook_dir );
     }
     my->_contact_db.open( abook_dir / "contact_db" );
     my->_address_index.open( abook_dir / "address_index" );

  } FC_RETHROW_EXCEPTIONS( warn, "", ("directory", abook_dir) ) }

  std::vector<std::string> addressbook::get_known_bitnames()const
  {
      std::vector<std::string> known_bitnames;
      auto itr = my->_contact_db.begin();
      while( itr.valid() )
      {
         known_bitnames.push_back(itr.key());
         ++itr;
      }
      return known_bitnames;
  }

  fc::optional<contact> addressbook::get_contact_by_bitname( const std::string& bitname_id )const
  { try {
      fc::optional<contact> con;
      auto itr = my->_contact_db.find(bitname_id);
      if( itr.valid() ) con = itr.value();
      return con;
  } FC_RETHROW_EXCEPTIONS( warn, "", ("bitname_id", bitname_id) ) }

  std::string addressbook::get_bitname_by_address( const bts::address& bitname_address )const
  { try {
      return my->_address_index.fetch( bitname_address );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("bitname_address", bitname_address) ) }

  void    addressbook::store_contact( const contact& contact_param )
  { try {
      my->_contact_db.store( contact_param.bitname_id, contact_param );
      my->_address_index.store( bts::address(contact_param.send_msg_address), contact_param.bitname_id );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("contact", contact_param) ) }

} } // bts::addressbook
