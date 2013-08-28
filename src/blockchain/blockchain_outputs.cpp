#include <bts/blockchain/outputs.hpp>

namespace bts { namespace blockchain {

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
