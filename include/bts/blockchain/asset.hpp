#pragma once
#include <fc/uint128.hpp>
#include <fc/io/enum_type.hpp>

namespace bts { namespace blockchain {

  struct price;

  /**
   *  An asset is a fixed point number with
   *  64.64 bit precision.  This is used
   *  for accumalating dividends and 
   *  calculating prices on the built in exchange.
   */
  struct asset
  {
      enum type
      {
          bts      = 0,  // 0.001 = 1 BitShare (smallest storable unit)
          btc      = 1,
          gld      = 2,
          slv      = 3,
          usd      = 4,  // $0.001 = 1 BitUSD 
          count, // TODO: move this to the end, for now this will shorten print statements
          cny      = 5,
          gbp      = 6,
          eur      = 7, 
          jpy      = 8,  // Japan Yen 
          chf      = 9,  // Swiss Frank #5 world currency 
          aud      = 10, // Austrialia
          cad      = 11, // Canada 
          sek      = 12, // Sweedish Krona 
          hkd      = 13, // Hong Kong 
          wti      = 14, // Light Sweet Crude Oil
          iii      = 15, // value of 1 of 1 billion shares in Invictus Innovations, Inc
      };

      static const fc::uint128& one();

      asset(){}
      asset( const std::string& str );
      asset( uint64_t int_part, asset::type t );

      asset& operator += ( const asset& o );
      asset& operator -= ( const asset& o );

      operator std::string()const;
       
      fc::uint128_t amount;
      type          unit;
  };
  
  /**
   *  A price is the result of dividing 2 asset classes and has
   *  the fixed point format 64.64 and -1 equals infinite.
   */
  struct price
  {
      static const fc::uint128& one();
      static const fc::uint128& infinite();

      price() {}
      price( const std::string& s );
      operator std::string()const;

      fc::uint128_t ratio; // 64.64

      uint16_t asset_pair()const { return (uint16_t(quote_unit)<<8) | base_unit; }

      asset::type base_unit;
      asset::type quote_unit;
  };

  inline bool operator == ( const asset& l, const asset& r ) { return l.amount == r.amount; }
  inline bool operator != ( const asset& l, const asset& r ) { return l.amount != r.amount; }
  inline bool operator <  ( const asset& l, const asset& r ) { return l.amount <  r.amount; }
  inline bool operator >  ( const asset& l, const asset& r ) { return l.amount >  r.amount; }
  inline bool operator <= ( const asset& l, const asset& r ) { return l.unit == r.unit && l.amount <= r.amount; }
  inline bool operator >= ( const asset& l, const asset& r ) { return l.unit == r.unit && l.amount >= r.amount; }
  inline asset operator +  ( const asset& l, const asset& r ) { return asset(l) += r; }
  inline asset operator -  ( const asset& l, const asset& r ) { return asset(l) -= r; }

  inline bool operator == ( const price& l, const price& r ) { return l.ratio == r.ratio; }
  inline bool operator != ( const price& l, const price& r ) { return l.ratio == r.ratio; }
  inline bool operator <  ( const price& l, const price& r ) { return l.ratio <  r.ratio; }
  inline bool operator >  ( const price& l, const price& r ) { return l.ratio >  r.ratio; }
  inline bool operator <= ( const price& l, const price& r ) { return l.ratio <= r.ratio && l.asset_pair() == r.asset_pair(); }
  inline bool operator >= ( const price& l, const price& r ) { return l.ratio >= r.ratio && l.asset_pair() == r.asset_pair(); }

  /**
   *  A price will reorder the asset types such that the
   *  asset type with the lower enum value is always the
   *  denominator.  Therefore  bts/usd and  usd/bts will
   *  always result in a price measured in usd/bts because
   *  bitasset_type::bit_shares <  bitasset_type::bit_usd.
   */
  price operator / ( const asset& a, const asset& b );

  /**
   *  Assuming a.type is either the numerator.type or denominator.type in
   *  the price equation, return the number of the other asset type that
   *  could be exchanged at price p.
   *
   *  ie:  p = 3 usd/bts & a = 4 bts then result = 12 usd
   *  ie:  p = 3 usd/bts & a = 4 usd then result = 1.333 bts 
   */
  asset operator * ( const asset& a, const price& p );


  typedef fc::enum_type<uint8_t,asset::type> asset_type;

} } // bts::blockchain

namespace fc
{
   void to_variant( const bts::blockchain::asset& var,  variant& vo );
   void from_variant( const variant& var,  bts::blockchain::asset& vo );
   void to_variant( const bts::blockchain::price& var,  variant& vo );
   void from_variant( const variant& var,  bts::blockchain::price& vo );
}

#include <fc/reflect/reflect.hpp>
FC_REFLECT_ENUM( bts::blockchain::asset::type, 
  (bts) (btc) (gld) (slv) (usd) (cny) (gbp) (eur) (jpy) (chf) (aud) (cad) (sek) (hkd) (wti) (iii)
)

