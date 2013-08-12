#include <bts/addressbook/addressbook.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace addressbook {

  namespace detail 
  { 
     class addressbook_impl
     {
        public:

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
  {
     if( !fc::exists( abook_dir ) )
     {
        fc::create_directories( abook_dir );
     }
  }

  contact addressbook::get_contact_by_bitname( const std::string& bname )const
  {
      return contact();
  }

  void    addressbook::store_contact( const contact& contact_param )
  {

  }

} } // bts::addressbook
