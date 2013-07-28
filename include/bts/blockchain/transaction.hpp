#pragma once
#include <bts/units.hpp>
#include <bts/address.hpp>
#include <bts/proof_of_work.hpp>
#include <fc/crypto/elliptic.hpp>
#include <fc/crypto/sha224.hpp>
#include <fc/io/varint.hpp>
#include <fc/exception/exception.hpp>
#include <city.h>

namespace bts {

/**
 *  A reference to a transaction and output index.
 */
struct output_reference
{
  output_reference():output_idx(0){}
  fc::uint128       trx_hash;   // the hash of a transaction.
  uint8_t           output_idx; // the output index in the transaction trx_hash
  friend bool operator==( const output_reference& a, const output_reference& b )
  {
     return a.trx_hash == b.trx_hash && a.output_idx == b.output_idx;
  }
  friend bool operator!=( const output_reference& a, const output_reference& b )
  {
     return !(a==b);
  }
};

enum claim_type
{
   /** basic claim by single address */
   null_claim_type  = 0,
   claim_by_address = 1,
   num_claim_types
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
    template<typename InputType>
    trx_input( const InputType& t, const ouptut_ref& src )
    :output_ref(src),output_type( InputType::type )
    {
       input_data = fc::raw::pack(t);
    }

    template<typename InputType>
    InputType as()const
    {
       FC_ASSERT( InputType::type == output_type );
       return fc::raw::unpack<InputType>(input_data);
    }

    output_reference   output_ref;
    claim_type         output_type;
    std::vector<char>  input_data;
};


/**
 *   An input that references an output that can be claimed by
 *   an address.  A compact signature is used that when combined
 *   with the hash of the transaction containing this input will
 *   allow the public key and therefore address to be discovered and
 *   validated against the output claim function.
 */
struct claim_by_address_input 
{
   static const claim_type type = claim_type::claim_by_address;
   fc::ecc::compact_signature address_sig;
};

struct claim_by_address_output
{
   static const claim_type type = claim_type::claim_by_address;
   uint160  address_sig; // hash of public key
};

struct claim_by_bid_output
{
   uint160                                ask_address; // where to send ask_unit
   fc::enum_type<uint8_t,bitasset_type>   ask_unit;    // the unit we are asking for
   fc::uint128                            ask_ppu;     // price per ask unit
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
    trx_output( const ClaimType& t, const fc::uint128& a, bitasset_type u )
    :amount(a),unit(u)
    {
       claim_func = ClaimType::type;
       claim_data = fc::raw::pack(t);
    }

    template<typename ClaimType>
    ClaimType as()const
    {
       FC_ASSERT( unit == ClaimType::type );
       return fc::raw::unpack<ClaimType>(claim_data);
    }

    trx_output(){}

    fc::uint128                                 amount;
    fc::enum_type<uint8_t,bitasset_type>        unit;
    fc::enum_type<fc::unsigned_int,claim_type>  claim_func;
    std::vector<char>                           claim_data;
};


/**
 *  @brief maps inputs to outputs.
 */
struct transaction
{
   fc::unsigned_int             version;      ///< trx version number
   fc::unsigned_int             valid_after;  ///< trx is only valid after block num, 0 means always valid
   fc::unsigned_int             valid_blocks; ///< number of blocks after valid after that this trx is valid, 0 means always valid
   std::vector<trx_input>       inputs;
   std::vector<trx_output>      outputs;
};

struct signed_transaction : public transaction
{
    /** @return the addresses that have signed this trx */
    std::vector<uint160> get_signed_addresses()const
    std::vector<fc::ecc::compact_signature> sigs;
};

} // namespace bts

namespace fc
{
  namespace raw 
  {
    template<typename Stream> 
    inline void pack( Stream& s, const bts::claim_type& v )
    {
       pack( s, char(v) );
    }
    template<typename Stream> 
    inline void unpack( Stream& s, bts::claim_type& v )
    {
       uint8_t o;
       unpack( s, o );
       v = bts::claim_type(o);
       FC_ASSERT( o < bts::num_claim_types )
    }
  }
}

FC_REFLECT( bts::output_reference, (trx_hash)(output_idx) )
FC_REFLECT_ENUM( bts::claim_type, (null_claim_type)(claim_by_address)(num_claim_types) )
FC_REFLECT( bts::trx_input, (output_ref) )
FC_REFLECT_DERIVED( bts::trx_input_by_address, (bts::trx_input), (address_sig) );
FC_REFLECT( bts::trx_output, (amount)(unit)(claim_func) )
FC_REFLECT_DERIVED( bts::trx_output_by_address, (bts::trx_output), (claim_address)(lock_time) )
FC_REFLECT( bts::generic_trx_in, (in_type)(data) )
FC_REFLECT( bts::generic_trx_out, (out_type)(data) )
FC_REFLECT( bts::transaction, (version)(inputs)(outputs) )
FC_REFLECT_DERIVED( bts::signed_transaction, (bts::transaction), (sigs) )

namespace std
{
  template<typename T>
  struct hash;

  template<>
  struct hash<bts::output_reference>
  {
     std::size_t operator()( const bts::output_reference& r )const
     {
        return CityHash64( (char*)&r, sizeof(r) );
     }
  };
}
