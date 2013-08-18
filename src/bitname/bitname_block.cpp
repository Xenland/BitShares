#include <bts/bitname/bitname_block.hpp>
#include <bts/difficulty.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/io/raw.hpp>
#include <fc/crypto/city.hpp>
#include <algorithm>

#include <fc/log/logger.hpp>

namespace bts { namespace bitname {

  name_id_type  name_header::id()const
  {
    name_id_type::encoder enc;
    auto d = fc::raw::pack(*this);
    return enc.result();
  }

  short_name_id_type name_header::short_id()const
  {
     auto long_id = id();
     return fc::city_hash64( (char*)&long_id, sizeof(long_id) );
  }


  uint64_t name_header::difficulty()const
  {
      return bts::difficulty(id());
  }

  uint64_t name_block::block_difficulty()const
  {
     uint64_t sum = 0;
     for( auto itr = registered_names.begin(); itr != registered_names.end(); ++itr )
     {
       sum += itr->difficulty( prev ); 
     }
     if( sum > 0 )
     {
         return difficulty() + sum;
     }
     return difficulty() / 2;
  }

  name_trxs_hash_type name_block::calc_trxs_hash()const
  {
     fc::sha512::encoder enc;
     fc::raw::pack( prev );
     fc::raw::pack( registered_names );
     auto result = enc.result();
     // city hash isn't crypto secure, but its input is sha512 which is.
     // we use city to compress the hash for bandwidth purposes
     return fc::city_hash128( (char*)&result, sizeof(result) );
  }

  /** helper method */
  name_id_type name_trx::id( const name_id_type& prev )const
  {
    return name_header( *this, prev ).id();
  }
  /** helper method */
  short_name_id_type name_trx::short_id( const name_id_type& prev )const
  {
    return name_header( *this, prev ).short_id();
  }

  uint64_t name_trx::difficulty( const name_id_type& prev )const
  {
    return name_header( *this, prev ).difficulty();
  }

  uint64_t min_name_difficulty() 
  {
      return 1;
  }

  name_block create_genesis_block()
  {
     name_block genesis;
     genesis.utc_sec = fc::time_point_sec(fc::time_point::from_iso_string( "20130814T000000" ));
     genesis.name_hash = 0;
     genesis.key = fc::ecc::private_key::regenerate(fc::sha256::hash( "genesis", 7)).get_public_key();
     return genesis;
  }

} }
