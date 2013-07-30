#pragma once
#include <bts/address.hpp>
#include <bts/small_hash.hpp>
#include <bts/blockchain/asset.hpp>
#include <fc/time.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/io/enum_type.hpp>

namespace bts { namespace blockchain {

enum claim_type_enum
{
   /** basic claim by single address */
   null_claim_type      = 0,
   claim_by_signature   = 1, ///< someone signs with an address
   claim_by_bid         = 2, ///< someone makes an acceptable bid
   claim_by_long        = 3, ///< someone agrees to go long against a short
   claim_by_cover       = 4, ///< someone covers a short, freeing collateral
   claim_by_opt_execute = 5, ///< someone executes an option
   claim_by_multi_sig   = 6, ///< N of M signatures required
   claim_by_escrow      = 7, ///< claimable with 2 of 3 signatures, 3rd signature can only split between 1&2
   claim_by_password    = 8, ///< used for cross-chain trading
   num_claim_types
};

typedef fc::enum_type<fc::unsigned_int,claim_type_enum> claim_type;


/**
 *   An input that references an output that can be claimed by
 *   an address.  A compact signature is used that when combined
 *   with the hash of the transaction containing this input will
 *   allow the public key and therefore address to be discovered and
 *   validated against the output claim function.
 */
struct claim_by_signature_input 
{
   static const claim_type_enum type = claim_type_enum::claim_by_signature;
//   fc::ecc::compact_signature address_sig;
};

struct claim_by_signature_output
{
   static const claim_type_enum type = claim_type_enum::claim_by_signature;
   claim_by_signature_output( const address& a ):owner(a){}
   claim_by_signature_output(){}
   address  owner; // checksummed hash of public key
};


/** 
 *  The output of the claim trx specifies the amount of a 
 *  specified asset unit we are offering in exchange for 
 *  the ask unit where the price per unit specifies the
 *  minimum (or maximum) exchange rate depending upon whether
 *  the ask_unit is the base or quote unit of the price.
 **/
struct claim_by_bid_output
{
   static const claim_type_enum type = claim_type_enum::claim_by_bid;
   address                           pay_address; // where to send ask_unit (or cancel sig)
   price                             ask_price;     // price base per unit
};

/**
 *  There are only two ways to claim a bid output, as part of a valid market
 *  transaction occuring below the ask price per unit or as part of a cancel
 *  operation.
 */
struct claim_by_bid_input
{
   static const claim_type_enum type = claim_type_enum::claim_by_bid;
//   fc::ecc::compact_signature  cancel_sig;  ///< signed by claim_by_bid_output::pay_address
};

/**
 *  This output may be spent via signature (to cancel) or via a market action
 *  of a miner.
 *
 *  Assumptions:
 *     trx_output.unit   ==  bts  
 *     trx_output.amount == amount of bts held as collateral
 *     ask_price         == price to convert to usd
 */
struct claim_by_long_output
{
   static const claim_type_enum type = claim_type_enum::claim_by_long;
   uint64_t                          min_trade;   ///< measured in bts to accept this order
   address                           pay_address; ///< where to send ask_unit (or cancel sig)
   price                             ask_price;   ///< price per unit (base must be bts)
};

/**
 *  There are only two ways to claim a short output, cancel it or take a long position by
 *  pairing it against a matching bid.  
 *
 *  Creates a negitive BTS input and positive USD input
 *
 */
struct claim_by_long_input
{
   static const claim_type_enum type = claim_type_enum::claim_by_long;
//   fc::ecc::compact_signature  cancel_sig;  ///< signed by claim_by_short_output::pay_address
};

/**
 *  Given a transaction that destroys payoff_amount of payoff_unit and is
 *  signed by owner, then this output can be spent.  Alternatively, the
 *  owner could transfer the short position to a new owner.
 *
 *  Assumptions:
 *    trx_output.unit = bts
 *    trx_output.amount = total collateral held
 *
 *    payoff_amount / unit counts as a 'negative input' to the transaction.
 */
struct claim_by_cover_output
{
   static const claim_type_enum type = claim_type_enum::claim_by_cover;
   asset::type    payoff_unit;
   uint64_t       payoff_amount;
   bts::address   owner;
};

/**
 *  With this signature the cover output may be either redeemed or the
 *  cover_output transferred to a new owner. 
 */
struct claim_by_cover_input
{
   static const claim_type_enum type = claim_type_enum::claim_by_cover;
//   fc::ecc::compact_signature owner_sig;
};


/**
 *  Options work as follows:
 *     Seller creates an unsigned transaction that contains the
 *            inputs necessary to fund the collateral and specifies 
 *            an call option output containing the strike unit / amount / time 
 *            and leaves the optionee blank.
 *     Seller creates an output that pays the desired fee to the seller.
 *          
 *     Buyer  adds their address to the optionee field
 *     Buyer  adds inputs necessary to fund both the trx fee and option fee 
 *     Buyer  signs the transaction, but it is not a valid transaction yet
 *            because it has inputs that require the seller's signature.
 *     Buyer  sends signed option to the seller
 *
 *     Seller reviews the terms, signs it, and broadcasts it to the network.
 *    
 *     At all times the trx has an expiration time where if the seller does
 *     not sign nor broadcast then the buyer can know they are safe to spend
 *     their money elsewhere. 
 *
 *     The buyer can cancel the purchase by double spending their inputs before the
 *     seller can broadcast.
 */


/**
 *  This output may be claimed by the optionor after the first block with a timestamp
 *  after the expire_time or by the optionee at any point prior to the expire_time provided
 *  it is part of a transaction that pays strike_amount of strike_unit to the optionor. 
 *
 *  Assumptions:
 *    trx.output.unit = the unit that may be bought provided strike unit/amount are provided
 *    trx.output.amount = the amount that may be bought
 *    If the option is exersized, the dividends earned go to the person who claims it.
 *
 *    Option may be rolled forward by either the optionor or optioneee, failure to
 *    roll the option forward results in the same 5% fee plus dividend forfeiture as
 *    any other output balance.
 */
struct claim_by_opt_execute_output
{
   static const claim_type_enum type = claim_type_enum::claim_by_opt_execute;
   address             optionor; // who to pay for this option (also who may cancel this offer)
   fc::time_point_sec  expire_time;   
   asset_type          strike_unit; 
   uint64_t            strike_amount; 
   address             optionee; // who owns the right to this option.
};

struct claim_by_opt_execute_input
{
   static const claim_type_enum type = claim_type_enum::claim_by_opt_execute;
// this signature is for the entire trx, and not just the input
//   fc::ecc::compact_signature   sig; // either optionor or optionee
};

#if 0 // kept for future escrow extention
struct escrow_terms
{
   enum expire_options 
   {
      pay_network = 0  // nash equalibrium ,
      pay_payee   = 1, // default, assume it worked
      pay_payer   = 2,
      split_pay   = 3
   };
   typedef fc::enum_type<uint8_t,expire_options> expire_option;

   fc::ecc::public_key contact;
   uint32_t            expire_blocks;   ///< blocks 
   expire_option       on_expire;       ///< who can claim the funds
   uint64_t            escrow_fee;      ///< bts paid to agent automatically (must be 2x min trx fee or more)
   uint16_t            surety_percent;  ///< extra deposit made by payor that is forfeited in case of default,
                                          // payor must post this before they can open a dispute.
                                          // loser of dispute pays their surety to the escrow agent.
   uint32_t            resolve_blocks;  ///< blocks the agent has to resolve the dispute before an appeal can be filed automatically
   uint160             other;           ///< other terms not managed by the blockchain
};
#endif 

struct claim_by_escrow_output
{
    static const claim_type_enum type = claim_type_enum::claim_by_opt_execute;
    uint160 agreement;    // hash of any agreement between payee and payor
    uint160 agent_terms;  // hash of escrow terms published by the agent
    address agent;        // agent must be registered with the network.
    address payee;
    address payor;
};

struct claim_by_escrow_input
{
    static const claim_type_enum type = claim_type_enum::claim_by_opt_execute;
};

/**
 *  This output can be claimed if sighed by payer & payee or
 *  signed by payee and the proper password is provided.
 */
struct claim_by_password_output
{
    static const claim_type_enum type = claim_type_enum::claim_by_password;
    address          payer;
    address          payee;
    fc::ripemd160    hashed_password;
};

/**
 *   Provide the password such that ripemd160( password ) == claim_by_password_output::hashed_password
 */
struct claim_by_password_input
{
    static const claim_type_enum type = claim_type_enum::claim_by_password;
    fc::uint128     password; ///< random number generated for cross chain trading
};


/**
 *  Used for multi-sig transactions that require N of M addresses to sign before
 *  the output can be spent.
 */
struct claim_by_multi_sig_output
{
    static const claim_type_enum type = claim_type_enum::claim_by_multi_sig;
    fc::unsigned_int    required;
    std::vector<address> addresses;
};


} } // bts::blockchain

FC_REFLECT_ENUM( bts::blockchain::claim_type_enum, 
    (null_claim_type)
    (claim_by_signature)
    (claim_by_bid)
    (claim_by_long)
    (claim_by_cover)
    (claim_by_opt_execute)
    (claim_by_multi_sig)
    (claim_by_escrow)
    (claim_by_password)
    )

FC_REFLECT( bts::blockchain::claim_by_signature_output, (owner) )
FC_REFLECT( bts::blockchain::claim_by_bid_output, (pay_address)(ask_price) )
FC_REFLECT( bts::blockchain::claim_by_long_output, (pay_address)(ask_price) )
FC_REFLECT( bts::blockchain::claim_by_cover_output, (payoff_unit)(payoff_amount)(owner) )
FC_REFLECT( bts::blockchain::claim_by_opt_execute_output, (optionor)(expire_time)(strike_unit)(strike_amount)(optionee) )
FC_REFLECT( bts::blockchain::claim_by_escrow_output, (agreement)(agent_terms)(agent)(payee)(payor) )
FC_REFLECT( bts::blockchain::claim_by_multi_sig_output, (required)(addresses) )
FC_REFLECT( bts::blockchain::claim_by_password_output, (payer)(payee)(hashed_password) )


FC_REFLECT( bts::blockchain::claim_by_signature_input,    BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_bid_input,          BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_long_input,         BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_cover_input,        BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_opt_execute_input,  BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_escrow_input,       BOOST_PP_SEQ_NIL )
FC_REFLECT( bts::blockchain::claim_by_password_input,     (password)       )

