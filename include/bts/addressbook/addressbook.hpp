#pragma once
#include <bts/addressbook/contact.hpp>

namespace fc { class path; }

namespace bts { namespace addressbook {

  namespace detail { class addressbook_impl; }

  /**
   *  Provides indexes for effecient lookup of contacts
   *  and abstracts the storage of the addressbook on
   *  disk.
   */
  class addressbook 
  {
     public:
        addressbook();
        ~addressbook();

        void open( const fc::path& abook_dir );

        contact get_contact_by_bitname( const std::string& bname )const;
        void    store_contact( const contact& c );

     private:
        std::unique_ptr<detail::addressbook_impl> my;
  };

} } // bts::addressbook
