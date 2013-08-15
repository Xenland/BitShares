#include <bts/bitname/bitname_block.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/io/raw.hpp>
#include <algorithm>

namespace bts { namespace bitname {
  mini_pow  name_header::id()const
  {
    auto d = fc::raw::pack(*this);
    return mini_pow_hash(d.data(),d.size());
  }


  uint64_t name_block::calc_difficulty()const
  {
     fc::bigint max_pow = to_bigint( mini_pow_max() );
     int64_t my_difficulty = (max_pow / to_bigint( id() )).to_int64();

     if( registered_names.size() == 0 ) 
        return my_difficulty;

     std::vector<uint64_t> difficulties(registered_names.size() );
     for( uint32_t i = 0; i < registered_names.size(); ++i )
     {
        difficulties[i] = (max_pow / to_bigint( registered_names[i].id(prev)  )).to_int64();
     }

     uint64_t median_pos = difficulties.size() / 2;
     std::nth_element( difficulties.begin(), difficulties.begin() + difficulties.size() / 2, difficulties.end() );
     auto median = difficulties[median_pos];

     return my_difficulty + median * (difficulties.size()+1);
  }

  mini_pow name_block::calc_merkle_root()const
  {
     if( registered_names.size() == 0 ) return mini_pow();
     if( registered_names.size() == 1 ) return registered_names.front().id(prev);

     std::vector<mini_pow> layer_one;
     for( auto itr = registered_names.begin(); itr != registered_names.end(); ++itr )
     {
       layer_one.push_back(itr->id(prev));
     }
     std::vector<mini_pow> layer_two;
     while( layer_one.size() > 1 )
     {
        if( layer_one.size() % 2 == 1 )
        {
          layer_one.push_back( mini_pow() );
        }

        static_assert( sizeof(mini_pow[2]) == 20, "validate there is no padding between array items" );
        for( uint32_t i = 0; i < layer_one.size(); i += 2 )
        {
            layer_two.push_back(  mini_pow_hash( layer_one[i].data, 2*sizeof(mini_pow) ) );
        }

        layer_one = std::move(layer_two);
     }
     return layer_one.front();
  }

  /** helper method */
  mini_pow name_trx::id( const mini_pow& prev )const
  {
    return name_header( *this, prev ).id();
  }
  name_block create_genesis_block()
  {
     name_block genesis;
     genesis.utc_sec = fc::time_point_sec(fc::time_point::from_iso_string( "20130814T000000" ));
     genesis.name_hash = 0;
     genesis.key = fc::ecc::private_key::regenerate(fc::sha256::hash( "genesis", 7)).get_public_key();
     genesis.mroot = genesis.calc_merkle_root();
     return genesis;
  }

} }
