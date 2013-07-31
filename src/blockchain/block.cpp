#include <bts/blockchain/block.hpp>
#include <bts/proof_of_work.hpp>
#include <bts/config.hpp>
#include <bts/small_hash.hpp>
#include <fc/io/raw.hpp>
#include <fc/reflect/variant.hpp>
namespace fc 
{

   typedef fc::array<bts::blockchain::asset_issuance,bts::blockchain::asset::type::count> issuance_type; 
   void to_variant( const issuance_type& var,  variant& vo )
   {
      mutable_variant_object obj;
      for( uint32_t i =  0; i < var.size(); ++i )
      {
          obj[fc::reflector<bts::blockchain::asset::type>::to_string( i )] = variant( var.at(i) ); 
      }
      vo = std::move(obj);
   }
   void from_variant( const variant& var,  issuance_type& vo )
   {

   }
}

namespace bts { namespace blockchain  {

  // 1 BTC = 100,000,000 satoshi 
  // 21,000,000  * 100,000,000
  // 2,100,000,000,000,000 
  // 1,000,000,000,000.000 == BitShare Supply 
  uint64_t calculate_mining_reward( uint32_t blk_num )
  {
      if( blk_num > BLOCKS_WITH_REWARD ) return 0;
//      if( blk_num == 0 ) return INITIAL_REWARD / 2;
      return (INITIAL_REWARD - (uint64_t(blk_num) * (REWARD_DELTA_PER_BLOCK)));
  }

  /**
   * Creates the gensis block and returns it.
   */
  trx_block create_genesis_block()
  {
    try {
      trx_block b;
      b.version    = 0;
      b.prev       = fc::sha224();
      b.block_num  = 0;
      b.timestamp  = fc::time_point::from_iso_string("20130730T054434");
      b.state_hash = b.state.digest();

      signed_transaction coinbase;
      coinbase.version = 0;
      coinbase.valid_after = 0;
      coinbase.valid_blocks = 0;

      coinbase.outputs.push_back( 
         trx_output( claim_by_signature_output( address("GmckPDdjQejZBP3t2gZqCqmEfi4") ), calculate_mining_reward(0)/2, asset::bts) );
      
      b.trxs.emplace_back( std::move(coinbase) );
 //     full_block fb = b;
      b.trx_mroot   = b.calculate_merkle_root();

      b.pow.branch_path.mid_states.push_back( b.digest() );
      return b;
    } FC_RETHROW_EXCEPTIONS( warn, "error creating gensis block" );
  }

  trx_block::operator full_block()const
  {
    full_block b( (const block&)*this );
    b.trx_ids.reserve( trxs.size() );
    for( auto itr = trxs.begin(); itr != trxs.end(); ++itr )
    {
      b.trx_ids.push_back( itr->id() );
    }
    return b;
  }


  uint160 trx_block::calculate_merkle_root()const
  {
     if( trxs.size() == 0 ) return uint160();
     if( trxs.size() == 1 ) return trxs.front().id();

     std::vector<uint160> layer_one;
     for( auto itr = trxs.begin(); itr != trxs.end(); ++itr )
     {
       layer_one.push_back(itr->id());
     }
     std::vector<uint160> layer_two;
     while( layer_one.size() > 1 )
     {
        if( layer_one.size() % 2 == 1 )
        {
          layer_one.push_back( uint160() );
        }

        static_assert( sizeof(uint160[2]) == 40, "validate there is no padding between array items" );
        for( uint32_t i = 0; i < layer_one.size(); i += 2 )
        {
            layer_two.push_back(  small_hash( (char*)&layer_one[i], 2*sizeof(uint160) ) );
        }

        layer_one = std::move(layer_two);
     }
     return layer_one.front();
  }

  uint160 block_state::digest()const
  {
     fc::sha512::encoder enc;
     fc::raw::pack( enc, *this );
     return small_hash( enc.result() );
  }

  /**
   *  Calculate the proof of work hash from the merkle root + nonce
   */
  mini_pow block_proof::proof_of_work()const
  {
     FC_ASSERT( pow.branch_path.mid_states.size() > 0 );
     FC_ASSERT( pow.branch_path.mid_states[0] == block_header::digest() );
     fc::sha256::encoder enc;
     fc::raw::pack( enc, pow.nonce );
     fc::raw::pack( enc, pow.branch_path.calculate_root() );
     return bts::proof_of_work( enc.result() );
  }

  /**
   *  @return the digest of the block header used to evaluate the proof of work
   */
  uint160 block_header::digest()const
  {
     fc::sha512::encoder enc;
     fc::raw::pack( enc, *this );
     return small_hash( enc.result() );
  }

} } // bts::blockchain
