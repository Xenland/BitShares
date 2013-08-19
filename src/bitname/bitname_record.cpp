#include <bts/bitname/bitname_record.hpp>
#include <fc/crypto/hex.hpp>

namespace bts { namespace bitname {
  uint64_t name_record::get_name_hash()const
  {
     uint64_t result = 0;
     fc::from_hex( name_hash, (char*)&result, sizeof(result) );
     return result;
  }

  void name_record::set_name_hash( uint64_t nhash )
  {
      name_hash = fc::to_hex( (char*)&nhash, sizeof(nhash) );
  }

} } // bts::bitname
