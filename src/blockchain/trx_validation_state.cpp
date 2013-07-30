#include <bts/blockchain/trx_validation_state.hpp>
#include <fc/reflect/variant.hpp>
namespace bts  { namespace blockchain { 

trx_validation_state::trx_validation_state( const signed_transaction& t, std::vector<meta_trx_input>&& in )
:trx(t),inputs(std::move(in)),balance_sheet( asset::count ),issue_sheet(asset::count)
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
     
     for( uint32_t i = 1; i < balance_sheet.size(); ++i )
     {
        if( !balance_sheet[i].is_balanced() )
        {
            FC_THROW_EXCEPTION( exception, "input value ${in} does not match output value ${out}",
                               ("in", std::string(balance_sheet[i].in))( "out", std::string(balance_sheet[i].out) ) );
                           
        }
     }
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


void trx_validation_state::validate_signature( const meta_trx_input& in )
{
}
void trx_validation_state::validate_bid( const meta_trx_input& in )
{
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

} }  // namespace bts::blockchain
