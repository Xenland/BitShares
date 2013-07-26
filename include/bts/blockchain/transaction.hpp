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
  fc::sha224        trx_hash;   // the hash of a transaction.
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
 *  reference an existing unspent output which
 *  it can do in one of two ways:
 *
 *  1) provide the index in the deterministic unspent output
 *     table, or provide the hash of the transaction and the
 *     index of the output.
 *
 *  2) provide the hash of the transaction and the output index.
 *
 *  Any unspent output that is older than 24 hours can safely
 *  be addressed using the unique id assigned in the unspent
 *  output table.  Transactions that reference 'newer' outputs
 *  will have to reference the Transaction directly to survive
 *  a potential reorganization.
 */
struct trx_input
{
    output_reference   output_ref;
    claim_type         output_type;
};


/**
 *   An input that references an output that can be claimed by
 *   an address.  A compact signature is used that when combined
 *   with the hash of the transaction containing this input will
 *   allow the public key and therefore address to be discovered and
 *   validated against the output claim function.
 */
struct trx_input_by_address : public trx_input
{
   enum type_enum { type =  claim_type::claim_by_address };
   fc::ecc::compact_signature address_sig;
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
    trx_output( claim_type t)
    :amount(0),
     claim_func(t){}
    uint64_t       amount;
    bond_type      unit;
    claim_type     claim_func;
};

/**
 *  Used by normal bitcoin-style outputs spendable with
 *  the signature of the private key.
 */
struct trx_output_by_address : public trx_output
{
   enum type_enum { type =  claim_type::claim_by_address };
   trx_output_by_address()
   :trx_output( claim_type::claim_by_address ), lock_time(0){}

   uint32_t lock_time;
   address  claim_address;  // the address that can claim this input.
   friend bool operator==( const trx_output_by_address& a, const trx_output_by_address& b )
   {
      return a.claim_address == b.claim_address && (a.lock_time == b.lock_time);
   }
};


/**
 *  Holds any type of transaction input in the data field.
 */
struct generic_trx_in
{
  uint8_t           in_type;
  std::vector<char> data;
};

struct generic_trx_out
{
  uint8_t           out_type;
  std::vector<char> data;
};

/**
 *  @brief maps inputs to outputs.
 */
struct transaction
{
   uint16_t                     version;
   uint32_t                     expire_block;
   std::vector<generic_trx_in>  inputs;
   std::vector<generic_trx_out> outputs;
};

struct signed_transaction : public transaction
{
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
