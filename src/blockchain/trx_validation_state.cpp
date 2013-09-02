#include <bts/blockchain/trx_validation_state.hpp>
#include <bts/config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>
#include <limits>

#include <fc/log/logger.hpp>

namespace bts  { namespace blockchain { 

trx_validation_state::trx_validation_state( const signed_transaction& t, blockchain_db* d, bool enf, uint32_t h )
:trx(t),balance_sheet( asset::count ),issue_sheet(asset::count),db(d),enforce_unspent(enf),ref_head(h)
{ 
  inputs  = d->fetch_inputs( t.inputs, ref_head );
  if( ref_head == std::numeric_limits<uint32_t>::max()  )
  {
    ref_head = d->head_block_num();
  }

  for( auto i = 0; i < asset::count; ++i )
  {
    balance_sheet[i].in.unit  = (asset::type)i;
    balance_sheet[i].out.unit = (asset::type)i;
  }
}

void trx_validation_state::validate()
{
  try
  {
     FC_ASSERT( trx.inputs.size() == inputs.size() );
     
     if( enforce_unspent )
     {
       for( uint32_t i = 0; i < inputs.size(); ++i )
       {
          if( inputs[i].meta_output.is_spent() )
          {
             FC_THROW_EXCEPTION( exception, 
                "input [${iidx}] = references output which was already spent",
                ("iidx",i)("input",trx.inputs[i])("output",inputs[i]) );
          }
       }
     }
     
     for( uint32_t i = 0; i < inputs.size(); ++i )
     {
       try {
         validate_input( inputs[i] ); 
       } FC_RETHROW_EXCEPTIONS( warn, "error validating input ${i}", ("i",i) );
     }

     for( uint32_t i = 0; i < trx.outputs.size(); ++i )
     {
       try {
         validate_output( trx.outputs[i] ); 
       } FC_RETHROW_EXCEPTIONS( warn, "error validating output ${i}", ("i",i) );
     }
     
     for( uint32_t i = 1; i < balance_sheet.size(); ++i )
     {
        if( !balance_sheet[i].is_balanced() )
        {
            FC_THROW_EXCEPTION( exception, "input value ${in} does not match output value ${out}",
                               ("in", std::string(balance_sheet[i].in))( "out", std::string(balance_sheet[i].out) ) );
                           
        }
     }

     if( !signed_addresses.size() )
     {
        signed_addresses =  trx.get_signed_addresses();
     }
     std::vector<address> missing;
     for( auto itr  = required_sigs.begin(); itr != required_sigs.end(); ++itr )
     {
        if( signed_addresses.find( *itr ) == signed_addresses.end() )
        {
           missing.push_back( *itr );
        }
     }
     if( missing.size() )
     {
        FC_THROW_EXCEPTION( exception, "missing signatures for ${addresses}", ("addresses", missing) );
     }

     // TODO: what should we do about trxs that include extra, unnecessary signatures
     // I cannot just compare required sigs to actual sigs because some trx such as
     // multisig, escrow, etc may optionally include N of M signatures.  Somewhere I should
     // count the total sigs required and then compare to actual number of sigs provided to
     // serve as the upper limit

  } FC_RETHROW_EXCEPTIONS( warn, "error validating transaction", ("state", *this)  );

} // validate 


void trx_validation_state::validate_input( const meta_trx_input& in )
{
     switch( in.output.claim_func )
     {
        case claim_by_signature:
          validate_signature( in );
          return;
        case claim_by_bid:
          validate_bid( in );
          return;
        case claim_by_long:
          validate_long( in );
          return;
        case claim_by_cover:
          validate_cover( in );
          return;
        case claim_by_opt_execute:
          validate_opt( in );
          return;
        case claim_by_multi_sig:
          validate_multi_sig( in );
          return;
        case claim_by_escrow:
          validate_escrow( in );
          return;
        case claim_by_password:
          validate_password( in );
          return;
        default:
          FC_THROW_EXCEPTION( exception, "unsupported claim function ${f}", ("f", in.output.claim_func ) );
     }
} // validate_input


void trx_validation_state::validate_output( const trx_output& out )
{
     FC_ASSERT( out.unit < asset::count );
     FC_ASSERT( out.amount < MAX_BITSHARE_SUPPLY ); // some sanity checks here
     switch( out.claim_func )
     {
        case claim_by_signature:
          validate_signature( out );
          return;
        case claim_by_bid:
          validate_bid( out );
          return;
        case claim_by_long:
          validate_long( out );
          return;
        case claim_by_cover:
          validate_cover( out );
          return;
        case claim_by_opt_execute:
          validate_opt( out );
          return;
        case claim_by_multi_sig:
          validate_multi_sig( out );
          return;
        case claim_by_escrow:
          validate_escrow( out );
          return;
        case claim_by_password:
          validate_password( out );
          return;
        default:
          FC_THROW_EXCEPTION( exception, "unsupported claim function ${f}", ("f", out.claim_func ) );
     }
} // validate_output

void trx_validation_state::validate_signature( const trx_output& o )
{
   auto cbs = o.as<claim_by_signature_output>();
   ilog( "${cbs}", ("cbs",cbs));
   FC_ASSERT( cbs.owner != address() );
   asset out(o.amount,o.unit);

   balance_sheet[(asset::type)o.unit].out += out;
   
}
void trx_validation_state::validate_bid( const trx_output& o )
{
   auto bid = o.as<claim_by_bid_output>();
   FC_ASSERT( bid.ask_price.ratio != fc::uint128(0) );
   FC_ASSERT( bid.pay_address != address() );
   FC_ASSERT( bid.ask_price.base_unit == o.unit ||
              bid.ask_price.quote_unit == o.unit );
   FC_ASSERT( bid.ask_price.base_unit != bid.ask_price.quote_unit );
   FC_ASSERT( bid.ask_price.base_unit.value < bid.ask_price.quote_unit.value );

   balance_sheet[(asset::type)o.unit].out += asset(o.amount,o.unit);
}
void trx_validation_state::validate_long( const trx_output& o )
{
   auto long_claim = o.as<claim_by_long_output>();
   FC_ASSERT( long_claim.ask_price.ratio != fc::uint128(0) );
   FC_ASSERT( long_claim.ask_price.base_unit != long_claim.ask_price.quote_unit );
   FC_ASSERT( long_claim.ask_price.base_unit.value < long_claim.ask_price.quote_unit.value );

   balance_sheet[(asset::type)o.unit].out += asset(o.amount,o.unit);
}
void trx_validation_state::validate_cover( const trx_output& o )
{
   auto cover_claim = o.as<claim_by_cover_output>();
   balance_sheet[(asset::type)o.unit].out += o.get_amount(); //asset(o.amount,o.unit);
   balance_sheet[(asset::type)cover_claim.payoff_unit].out -= cover_claim.get_payoff_amount();
}
void trx_validation_state::validate_opt( const trx_output& o )
{
   auto opt_claim = o.as<claim_by_opt_execute_output>();
}
void trx_validation_state::validate_multi_sig( const trx_output& o )
{
   auto multsig_claim = o.as<claim_by_multi_sig_output>();
}
void trx_validation_state::validate_escrow( const trx_output& o )
{
   auto escrow_claim = o.as<claim_by_escrow_output>();
}
void trx_validation_state::validate_password( const trx_output& o )
{
   auto password_claim = o.as<claim_by_password_output>();
}



/**
 *  Adds the owner to the required signature list
 *  Adds the balance to the trx balance sheet
 *  Adds dividends from the balance to the balance sheet
 *  Adds fee dividends to the fee total.
 *
 *  TODO: this input is also valid if it is 1 year old and an output exists
 *        paying 95% of the balance back to the owner.
 *
 *  TODO: what if the source is an unvested market order... it means the
 *        proceeds of this trx are also 'unvested'.  Perhaps we will have to
 *        propagate the vested state along with the trx, if any inputs are
 *        sourced from an unvested trx, the new trx is also 'unvested' until
 *        the most receint input is fully vested.
 */
void trx_validation_state::validate_signature( const meta_trx_input& in )
{
   try {
       auto cbs = in.output.as<claim_by_signature_output>();
       ilog( "${cbs}", ("cbs",cbs));
       required_sigs.insert( cbs.owner );

       asset output_bal( in.output.amount, in.output.unit );
       balance_sheet[(asset::type)in.output.unit].in += output_bal;

       dividend_fees  += db->calculate_dividend_fees( output_bal, in.source.block_num, ref_head );
       auto new_div = db->calculate_output_dividends( output_bal, in.source.block_num, ref_head );
       dividends      += new_div;

       elog( "dividends ${D}   new_div ${nd}    IN: ${IN}", ("D", dividends )("IN",in)("nd",new_div) );

       // dividends are always paid in bts
       balance_sheet[asset::bts].in += new_div;
   
   } FC_RETHROW_EXCEPTIONS( warn, "validating signature input ${i}", ("i",in) );
}


/**
 *  A bid input is a valid input in two cases:
 *
 *  1) Signed by owner
 *  2) A suitable output exists in trx that meets the requirements of the bid.
 *       - output paying proper proper asset to proper address at specified exchange rate
 *       - left-over-bid sent to new output with same terms.
 *       - accepted and change bids > min amount
 */
void trx_validation_state::validate_bid( const meta_trx_input& in )
{ try {
    auto cbb = in.output.as<claim_by_bid_output>();
   
    asset output_bal( in.output.amount, in.output.unit );
    balance_sheet[(asset::type)in.output.unit].in += output_bal;

    dividend_fees  += db->calculate_dividend_fees( output_bal, in.source.block_num, ref_head );
    auto new_div = db->calculate_output_dividends( output_bal, in.source.block_num, ref_head );
    dividends      += new_div;

    balance_sheet[asset::bts].in += new_div;


    // if the pay address has signed the trx, then that means this is a cancel request
    if( signed_addresses.find( cbb.pay_address ) != signed_addresses.end() )
    {
       // canceled orders can reclaim their dividends (assuming the order has been open long enough)
       balance_sheet[asset::bts].in += db->calculate_output_dividends( output_bal, in.source.block_num );
    }
    else // someone else accepted the offer based upon the terms of the bid.
    {
       // accepted bids pay their dividends to the miner, if there are any to speak of
       dividend_fees  += db->calculate_output_dividends( output_bal, in.source.block_num );

       // some orders may be split and thus result in
       // two outputs being generated... look for the split order first, then look
       // for the change!  Easy peesy..
       uint16_t split_order = find_unused_bid_output( cbb );
       if( split_order == output_not_found ) // must be a full order...
       {
         uint16_t sig_out   = find_unused_sig_output( cbb.pay_address, output_bal * cbb.ask_price  );
         FC_ASSERT( sig_out != output_not_found, " unable to find payment of ${asset} to ${pay_addr}", ("asset", output_bal*cbb.ask_price)("pay_addr",cbb.pay_address)("in",in) );
         mark_output_as_used( sig_out );
       }
       else // look for change, must be a partial order
       {
         mark_output_as_used( split_order );
         const trx_output& split_out = trx.outputs[split_order];
         auto split_claim = split_out.as<claim_by_bid_output>();

         FC_ASSERT( split_out.amount                      >= cbb.min_trade );
         FC_ASSERT( (in.output.amount - split_out.amount) >= cbb.min_trade );
         FC_ASSERT( split_out.unit   == in.output.unit                     );
         
         // the balance of the order that was accepted
         asset accepted_bal = asset( in.output.amount - split_out.amount, split_out.unit ) * cbb.ask_price; 

         // get balance of partial order, validate that it is greater than min_trade 
         // subtract partial order from output_bal and insure the remaining order is greater than min_trade
         // look for an output making payment of the balance to the pay address
         uint16_t sig_out   = find_unused_sig_output( cbb.pay_address, accepted_bal * cbb.ask_price );
         FC_ASSERT( sig_out != output_not_found );
         mark_output_as_used( sig_out );
       }
    }
    
} FC_RETHROW_EXCEPTIONS( warn, "validating bid input ${i}", ("i",in) ) }

/**
 *  Someone offering to go 'short' created an output that could be claimed
 *  by going long.   When taken as an input the output set must contain an
 *  unused cover output, along with the 
 */
void trx_validation_state::validate_long( const meta_trx_input& in )
{ try {
    auto long_claim = in.output.as<claim_by_long_output>();
    asset output_bal( in.output.amount, in.output.unit );
    balance_sheet[(asset::type)in.output.unit].in += output_bal;

    dividend_fees  += db->calculate_dividend_fees( output_bal, in.source.block_num, ref_head );
    auto new_div = db->calculate_output_dividends( output_bal, in.source.block_num, ref_head );
    dividends      += new_div;

    balance_sheet[asset::bts].in += new_div;

/*
    if( signed_addresses.find( long_claim.pay_address ) != signed_addresses.end() )
    {
        // canceled orders can reclaim their dividends (assuming the order has been open long enough)
        balance_sheet[asset::bts].in += db->calculate_output_dividends( output_bal, in.source.block_num );

    }
    else // someone else accepted the offer based on terms of the long
    {
        asset bal = output_bal; // TODO - split_order.bal
        dividend_fees  += db->calculate_output_dividends( output_bal, in.source.block_num );

        uint16_t split_order = find_unused_long_output( long_claim );
        if( split_order == output_not_found ) // must be a full order...
        {
            // TODO: what about multiple outputs that add up to output_bal*cbb.ask_price and are paid to pay_address?
            // why would we do that? There is no reason.
            uint16_t sig_out  = find_unused_sig_output( long_claim.pay_address, output_bal * cbb.ask_price  );
            FC_ASSERT( sig_out != output_not_found );
        }
        else // look for change, must be partial order
        {
            // TODO: mark the split_order as used... 

            // TODO eval uint16_t sig_out   = find_unused_sig_output( cbb.pay_address, bal * cbb.ask_price );
        }
    }
    */
} FC_RETHROW_EXCEPTIONS( warn, "", ("in",in) ) } // validate_long

void trx_validation_state::validate_cover( const meta_trx_input& in )
{
   auto cover_in = in.output.as<claim_by_cover_output>();
   
}

void trx_validation_state::validate_opt( const meta_trx_input& in )
{

}

void trx_validation_state::validate_multi_sig( const meta_trx_input& in )
{
}

void trx_validation_state::validate_escrow( const meta_trx_input& in )
{
}
void trx_validation_state::validate_password( const meta_trx_input& in )
{
}

void trx_validation_state::mark_output_as_used( uint16_t output_number )
{
  FC_ASSERT( output_number != output_not_found );
  FC_ASSERT( used_outputs.insert(output_number).second ); // make sure we don't mark it twice
}

uint16_t trx_validation_state::find_unused_sig_output( const address& owner, const asset& bal )
{
  auto rounded_amount = asset(bal.get_rounded_amount(),bal.unit);
  ilog( "find unused sig output ${o}  ${bal}", ("o", owner)("bal",bal) );
  for( uint32_t i = 0; i < trx.outputs.size(); ++i )
  {
    ilog( "${i}",("i",i) );
     if( used_outputs.find(i) == used_outputs.end() )
     {
      
    ilog( "${i}",("i",i) );
        if( trx.outputs[i].claim_func == claim_by_signature )
        {
    ilog( "${i}",("i",i) );
           if( trx.outputs[i].get_amount() == rounded_amount )
           {
    ilog( "${i}",("i",i) );
              if( trx.outputs[i].as<claim_by_signature_output>().owner == owner )
              {
    ilog( "${i}",("i",i) );
                 return i;
              }
           }
        }
     }
  }
  return output_not_found;
}

/**
 *  Find a bid that matches the pay_to_address and price, amount may be different because
 *  of partial orders.
 */
uint16_t trx_validation_state::find_unused_bid_output( const claim_by_bid_output& bid_claim )
{
  for( uint32_t i = 0; i < trx.outputs.size(); ++i )
  {
     if( used_outputs.find(i) != used_outputs.end() )
     {
        if( trx.outputs[i].claim_func == claim_by_bid )
        {
           if( trx.outputs[i].as<claim_by_bid_output>() == bid_claim )
           {
              return i;
           }
        }
     }
  }
  return output_not_found;
}

uint16_t trx_validation_state::find_unused_long_output( const claim_by_long_output& b )
{
  return output_not_found;
}
uint16_t trx_validation_state::find_unused_cover_output( const claim_by_cover_output& b )
{
  return output_not_found;
}


} }  // namespace bts::blockchain
