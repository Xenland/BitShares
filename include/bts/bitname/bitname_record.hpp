#pragma once
#include <fc/crypto/elliptic.hpp>
#include <fc/time.hpp>

namespace bts { namespace bitname {

  /**
   * Summary information about a name registered on the
   * bitname blockchain.
   */
  struct name_record
  {
     name_record()
     :revoked(false),age(0),repute(0){}

     /** handle to/from hex conversion */
     uint64_t get_name_hash()const;
     void     set_name_hash( uint64_t h );

     fc::time_point_sec          last_update; ///< the most recent update of this name
     fc::ecc::public_key_data    pub_key;     ///< the public key paired to this name.
     bool                        revoked;     ///< this name has been canceled, by its owner (invalidating the public key)
     uint32_t                    age;         ///< first block this name was registered in
     uint32_t                    repute;      ///< how many repute points have been earned by this name
     std::string                 name_hash;   ///< the unique hash assigned to this name, in hex format
     fc::optional<std::string>   name;        ///< if we know the name that generated the hash
  };

} } // bts::bitname

#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::bitname::name_record,
    (last_update)
    (pub_key)
    (revoked)
    (age)
    (repute)
    (name_hash)
    (name)
  )
