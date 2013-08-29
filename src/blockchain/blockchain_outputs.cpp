#include <bts/blockchain/outputs.hpp>

namespace bts { namespace blockchain {

   const claim_type_enum claim_by_signature_output::type    = claim_type_enum::claim_by_signature;
   const claim_type_enum claim_by_bid_output::type          = claim_type_enum::claim_by_bid;
   const claim_type_enum claim_by_long_output::type         = claim_type_enum::claim_by_long;
   const claim_type_enum claim_by_cover_output::type        = claim_type_enum::claim_by_cover;
   const claim_type_enum claim_by_opt_execute_output::type  = claim_type_enum::claim_by_opt_execute;
   const claim_type_enum claim_by_password_output::type     = claim_type_enum::claim_by_password;
   const claim_type_enum claim_by_escrow_output::type       = claim_type_enum::claim_by_password;
   const claim_type_enum claim_by_multi_sig_output::type    = claim_type_enum::claim_by_multi_sig;


   const claim_type_enum claim_by_signature_input::type    = claim_type_enum::claim_by_signature;
   const claim_type_enum claim_by_bid_input::type          = claim_type_enum::claim_by_bid;
   const claim_type_enum claim_by_long_input::type         = claim_type_enum::claim_by_long;
   const claim_type_enum claim_by_cover_input::type        = claim_type_enum::claim_by_cover;
   const claim_type_enum claim_by_opt_execute_input::type  = claim_type_enum::claim_by_opt_execute;
   const claim_type_enum claim_by_password_input::type     = claim_type_enum::claim_by_password;
   const claim_type_enum claim_by_multi_sig_input::type    = claim_type_enum::claim_by_multi_sig;

   bool claim_by_bid_output::is_ask( asset::type out_unit )const
   {
     return ask_price.quote_unit == out_unit;
   }

   bool claim_by_bid_output::is_bid( asset::type out_unit )const
   {
     return !is_ask(out_unit);
   }

   bool claim_by_bid_output::operator == ( const claim_by_bid_output& other )const
   {
      return  (other.pay_address == pay_address) &&
              (other.ask_price   == ask_price)   &&
              (other.min_trade   == min_trade);
   }

} } // bts::blockchain
