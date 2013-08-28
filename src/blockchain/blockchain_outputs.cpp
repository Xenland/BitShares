#include <bts/blockchain/outputs.hpp>

namespace bts { namespace blockchain {


   bool claim_by_bid_output::operator == ( const claim_by_bid_output& other )const
   {
      return  (other.pay_address == pay_address) &&
              (other.ask_price   == ask_price)   &&
              (other.min_order   == min_order);
   }

} } // bts::blockchain
