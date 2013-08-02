#include <bts/blockchain/trx_validation_state.hpp>
#include <bts/config.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>

#include <fc/log/logger.hpp>

namespace bts  { namespace blockchain { 

trx_validation_state::trx_validation_state( const signed_transaction& t, std::vector<meta_trx_input>&& in, blockchain_db* d )
:trx(t),inputs(std::move(in)),balance_sheet( asset::count ),issue_sheet(asset::count),db(d)
{ 
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
     
     for( uint32_t i = 0; i < inputs.size(); ++i )
     {
        if( inputs[i].meta_output.is_spent() )
        {
           FC_THROW_EXCEPTION( exception, 
              "input [${iidx}] = ${i} references output ${o} which was already spent here ${s}",
              ("iid",i)("i",trx.inputs[i])("o",inputs[i]) );
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

     std::unordered_set<address> sigs =  trx.get_signed_addresses();
     std::vector<address> missing;
     for( auto itr  = required_sigs.begin(); itr != required_sigs.end(); ++itr )
     {
        if( sigs.find( *itr ) == sigs.end() )
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
   FC_ASSERT( o.unit < asset::count );
   FC_ASSERT( o.amount < MAX_BITSHARE_SUPPLY ); // some sanity checs here
   balance_sheet[(asset::type)o.unit].out += asset(o.amount,o.unit);
}
void trx_validation_state::validate_bid( const trx_output& )
{
}
void trx_validation_state::validate_long( const trx_output& )
{
}
void trx_validation_state::validate_cover( const trx_output& )
{
}
void trx_validation_state::validate_opt( const trx_output& )
{
}
void trx_validation_state::validate_multi_sig( const trx_output& )
{
}
void trx_validation_state::validate_escrow( const trx_output& )
{
}
void trx_validation_state::validate_password( const trx_output& )
{
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

       dividend_fees  += db->calculate_dividend_fees( output_bal, in.source.block_num );

       // dividends are always paid in bts
       balance_sheet[asset::bts].in += db->calculate_output_dividends( output_bal, in.source.block_num );
   
   } FC_RETHROW_EXCEPTIONS( warn, "validating signature input ${i}", ("i",in) );
}


/**
 *  A bid transaction is a valid input in two cases:
 *
 *  1) Signed by owner
 *  2) A suitable output exists in trx that meets the requirements of the bid.
 */
void trx_validation_state::validate_bid( const meta_trx_input& in )
{
   try {
       auto cbb = in.output.as<claim_by_bid_output>();
       
       asset output_bal( in.output.amount, in.output.unit );
       balance_sheet[(asset::type)in.output.unit].in += output_bal;
       dividend_fees                                 += db->calculate_dividend_fees( output_bal, in.source.block_num );


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

          // find an claim_by_sig output paying ask_price to pay_address
          // there may be multiple outputs meeting this description... 
          // TODO... sort this out... some orders may be split and thus result in
          // two outputs being generated... look for the split order first, then look
          // for the change!  Easy peesy..
          
          uint16_t split_order = find_unused_bid_output( cbb );
          if( split_order == output_not_found ) // must be a full order...
          {
            uint16_t sig_out   = find_unused_sig_output( cbb.pay_address, output_bal * cbb.ask_price  );
            // TODO: mark the sig_out as used

          }
          else // look for change, must be a partial order
          {
            // TODO: mark the split_order as used... 

            // get balance of partial order, validate that it is greater than min_order 
            // subtract partial order from output_bal and insure the remaining order is greater than min_order
            // look for an output making payment of the balance to the pay address
            asset bal = output_bal; // TODO - split_order.bal
            uint16_t sig_out   = find_unused_sig_output( cbb.pay_address, bal * cbb.ask_price );
          }
       }
   } FC_RETHROW_EXCEPTIONS( warn, "validating bid input ${i}", ("i",in) );
}



void trx_validation_state::validate_long( const meta_trx_input& in )
{
}
void trx_validation_state::validate_cover( const meta_trx_input& in )
{
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

uint16_t trx_validation_state::find_unused_bid_output( const claim_by_bid_output& b )
{
  return output_not_found;
}
uint16_t trx_validation_state::find_unused_sig_output( const address& a, const asset& bal )
{
  return output_not_found;
}


} }  // namespace bts::blockchain
