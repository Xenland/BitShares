#pragma once
#include <bts/extended_address.hpp>

namespace bts { namespace addressbook {
  
  /**
   *  Ultimately this class is used to manage all information
   *  known about an individual contact.
   */
  struct contact 
  {
      contact()
      :version(0),next_send_trx_id(0){}
      
      /** just incase we update the contact datafields 
       * in future versions. 
       **/
      uint32_t    version;

      std::string first_name;
      std::string last_name;
      std::string company;

      /** bitname used by this contact (should line up with send_msg_address */
      std::string bitname_id; 

      /** Key used to encode private messages sent to this contact */
      fc::ecc::public_key     send_msg_address;
      /** channels this contact is expected to be listening on */
      std::vector<uint16_t>   send_msg_channels;

      /**
       *  Private key to which broadcast messages are
       *  sent.
       */
      fc::ecc::private_key    recv_broadcast_msg_address;

      /** channels on which this contact broadcasts */
      std::vector<uint16_t>   recv_broadcast_msg_channels;

      /** used to generate adresses for sending funds to this
       * contact.
       */
      extended_public_key     send_trx_address;

      /**
       *  Incremented everytime a new trx is created to
       *  this contact.
       */
      uint32_t                next_send_trx_id;

      /**
       *  Addresses that this uesr has the private
       *  keys to.  This address is given to the
       *  contact who can use these keys to 
       *  send us money.
       */
      extended_public_key     recv_trx_address;
  };

} } // bts::addressbook

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::addressbook::contact,
    (version)
    (first_name)
    (last_name)
    (company)
    (bitname_id)
    (send_msg_address)
    (send_msg_channels)
    (recv_broadcast_msg_address)
    (recv_broadcast_msg_channels)
    (send_trx_address)
    (recv_trx_address)
)
  
