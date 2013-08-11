#pragma once
#include <bts/blockchain/asset.hpp>
#include <bts/blockchain/outputs.hpp>
#include <bts/units.hpp>
#include <bts/address.hpp>
#include <bts/proof_of_work.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/io/varint.hpp>
#include <fc/exception/exception.hpp>

namespace bts { namespace blockchain {

/**
 *  A reference to a transaction and output index.
 */
struct output_reference
{
  output_reference():output_idx(0){}
  output_reference( const uint160& trx, uint8_t idx )
  :trx_hash(trx),output_idx(idx){}
  uint160  trx_hash;   // the hash of a transaction.
  uint8_t  output_idx; // the output index in the transaction trx_hash
  
  friend bool operator==( const output_reference& a, const output_reference& b )
  {
     return a.trx_hash == b.trx_hash && a.output_idx == b.output_idx;
  }
  friend bool operator!=( const output_reference& a, const output_reference& b )
  {
     return !(a==b);
  }
};

/**
 *  Defines the source of an input used 
 *  as part of a transaction.  If must first
 *  reference an existing unspent output and
 *  then provide the input data required to
 *  cause the claim function to evaluate true.
 */
struct trx_input
{
    trx_input(){}
    trx_input( const output_reference& src )
    :output_ref(src)
    { }

    template<typename InputType>
    trx_input( const InputType& t, const output_reference& src )
    :output_ref(src)
    {
       input_data = fc::raw::pack(t);
    }

    template<typename InputType>
    InputType as()const
    {
       return fc::raw::unpack<InputType>(input_data);
    }

    output_reference   output_ref;
    std::vector<char>  input_data;
};


/**
 *  Base of all trx outputs, derived classes will define the extra
 *  data associated with the claim_func.  The goal is to enable the
 *  output types to be 'fixed size' and thus each output type would
 *  have its own 'memory pool' in the chain state data structure. This
 *  has the added benefit of conserving space, separating bids/asks/
 *  escrow/and normal transfers into different memory segments and
 *  should give better memory performance.   
 */
struct trx_output
{
    template<typename ClaimType>
    trx_output( const ClaimType& t, uint64_t a, asset::type u )
    :amount(a),unit(u)
    {
       claim_func = ClaimType::type;
       claim_data = fc::raw::pack(t);
    }

    template<typename ClaimType>
    ClaimType as()const
    {
       FC_ASSERT( claim_func == ClaimType::type );
       return fc::raw::unpack<ClaimType>(claim_data);
    }

    trx_output():amount(0){}
    // NOTE: note this class has a custom to_variant method 
    //   that pretty prints the claim data for known types
    //   and this method must be updated if you change these
    //   fields.
    uint64_t                                    amount;
    asset_type                                  unit;
    claim_type                                  claim_func;
    std::vector<char>                           claim_data;
};


/**
 *  @brief maps inputs to outputs.
 *
 */
struct transaction
{
   fc::sha256                   digest()const;

   fc::unsigned_int             version;      ///< trx version number
   fc::unsigned_int             valid_after;  ///< trx is only valid after block num, 0 means always valid
   fc::unsigned_int             valid_blocks; ///< number of blocks after valid after that this trx is valid, 0 means always valid
   std::vector<trx_input>       inputs;
   std::vector<trx_output>      outputs;
};

struct signed_transaction : public transaction
{
    std::unordered_set<address>      get_signed_addresses()const;
    uint160                          id()const;
    void                             sign( const fc::ecc::private_key& k );

    std::unordered_set<fc::ecc::compact_signature> sigs;
};

} }  // namespace bts::blockchain

namespace fc {
   void to_variant( const bts::blockchain::trx_output& var,  variant& vo );
   void from_variant( const variant& var,  bts::blockchain::trx_output& vo );
};

namespace std {
  /**
   *  This is implemented to facilitate generation of unique
   *  sets of inputs.
   */
  template<>
  struct hash<bts::blockchain::output_reference>
  {
     size_t operator()( const bts::blockchain::output_reference& e )const
     {
        return fc::city_hash64( (char*)&e, sizeof(e) );
     }
  };

} // std

FC_REFLECT( bts::blockchain::output_reference, (trx_hash)(output_idx) )
FC_REFLECT( bts::blockchain::trx_input, (output_ref)(input_data) )
FC_REFLECT( bts::blockchain::trx_output, (amount)(unit)(claim_func)(claim_data) )
FC_REFLECT( bts::blockchain::transaction, (version)(valid_after)(valid_blocks)(inputs)(outputs) )
FC_REFLECT_DERIVED( bts::blockchain::signed_transaction, (bts::blockchain::transaction), (sigs) );

