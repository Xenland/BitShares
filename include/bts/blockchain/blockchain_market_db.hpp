#pragma once
#include <bts/blockchain/block.hpp>
#include <bts/blockchain/asset.hpp>
#include <fc/optional.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace blockchain {

  namespace detail { class market_db_impl; }

 /**
  *   Bids:  (offers to buy Base Unit with Quote Unit)
  *   Quote Unit, Base Unit  Price  UnspentOutput 
  *
  *   Asks:  (offers to sell Base Unit for Quote Unit ) includes open short orders when BaseUnit = BTS)
  *   Quote Unit, Base Unit  Price  UnspentOutput 
  *
  */
  struct market_order
  {
     market_order( const price& p, const output_reference& loc );
     market_order(){}
     asset_type        base_unit;
     asset_type        quote_unit;
     fc::uint128_t     ratio; // 64.64
     output_reference  location;

     price get_price()const;
  };
  bool operator < ( const market_order& a, const market_order& b );
  bool operator == ( const market_order& a, const market_order& b );
  
  /**
   *  Manages the current state of the market to enable effecient
   *  pairing of the highest bid with the lowest ask.
   */
  class market_db
  {
     public:
       market_db();
       ~market_db();

       void open( const fc::path& db_dir );
       void insert_bid( const market_order& m );
       void insert_ask( const market_order& m );
       void remove_bid( const market_order& m );
       void remove_ask( const market_order& m );

       /** @pre quote > base  */
       fc::optional<market_order> get_highest_bid( asset::type quote, asset::type base );
       /** @pre quote > base  */
       fc::optional<market_order> get_lowest_ask( asset::type quote, asset::type base );

     private:
       std::unique_ptr<detail::market_db_impl> my;
  };

} }  // bts::blockchain

FC_REFLECT( bts::blockchain::market_order, (base_unit)(quote_unit)(ratio)(location) );
