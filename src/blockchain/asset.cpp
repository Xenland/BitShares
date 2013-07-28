#include <bts/blockchain/asset.hpp>
#include <fc/exception/exception.hpp>
#include <fc/crypto/bigint.hpp>
#include <fc/log/logger.hpp>

/** more base 10 digits is beyond the precision of 64 bits */
#define BASE10_PRECISION  (100000000000000ll)


namespace bts { namespace blockchain {

  asset::asset( const std::string& s )
  {
    // TODO:  string to asset
  }

  asset::asset( uint64_t int_p, asset::type t )
  {
     amount = fc::uint128( int_p, 0 );
     unit = t;
  }

  asset::operator std::string()const
  {
     uint64_t integer     = amount.high_bits();
     fc::uint128 fraction( amount.low_bits() );
     fraction *= BASE10_PRECISION;
     fraction /= one();
     fraction += BASE10_PRECISION;

     if( fraction.to_uint64()  % 10 >= 5 )
     {
        fraction +=  10 - (fraction.to_uint64()  % 10);
        integer += ((fraction / BASE10_PRECISION) - 1).to_uint64();
     }

     std::string s = fc::to_string(integer);
     s += ".";
     std::string frac(fraction);
     s += frac.substr(1,frac.size()-2);

     s += " " + std::string(fc::reflector<asset::type>::to_string( unit ));
     return s;
  }



  const fc::uint128& asset::one()
  {
     static fc::uint128_t o = fc::uint128(1,0);
     return o;
  }

  asset& asset::operator += ( const asset& o )
  {
     FC_ASSERT( unit == o.unit );

     auto old = *this;
     amount += o.amount;

     if( amount < old.amount ) 
     {
       FC_THROW_EXCEPTION( exception, "asset addition overflowed  ${a} + ${b} = ${c}", 
                            ("a", old)("b",o)("c",*this) );
     }
     return *this;
  }

  asset& asset::operator -= ( const asset& o )
  {
     FC_ASSERT( unit == o.unit );
     auto old = *this;;
     amount -= o.amount;
     if( amount > old.amount ) 
     {
       FC_THROW_EXCEPTION( exception, "asset addition underflow  ${a} - ${b} = ${c}", 
                            ("a", old)("b",o)("c",*this) );
     }
     return *this;
  }

  const fc::uint128& price::one()
  {
     static fc::uint128_t o = fc::uint128(1,0);
     return o;
  }
  const fc::uint128& price::infinite()
  {
      static fc::uint128 i(-1);
      return i;
  }
  price::price( const std::string& s )
  {
    // TODO:  price to asset
  }

  price::operator std::string()const
  {
     uint64_t integer = ratio.high_bits();
     fc::uint128 one(1,0);

     fc::uint128 fraction(0,ratio.low_bits());
     fraction *= BASE10_PRECISION;
     fraction /= one;
     fraction += BASE10_PRECISION;
     if( fraction.to_uint64()  % 10 >= 5 )
     {
        fraction += 10 - (fraction.to_uint64()  % 10);
        integer += ((fraction / BASE10_PRECISION) - 1).to_uint64();
     }
     std::string s = fc::to_string(integer);
     s += ".";
     std::string frac(fraction);
     s += frac.substr(1,frac.size()-2);
     s += " " + std::string(fc::reflector<asset::type>::to_string( quote_unit ));
     s += "/" + std::string(fc::reflector<asset::type>::to_string( base_unit ));
     return s;
  }

  /**
   *  A price will reorder the asset types such that the
   *  asset type with the lower enum value is always the
   *  denominator.  Therefore  bts/usd and  usd/bts will
   *  always result in a price measured in usd/bts because
   *  bitasset_type::bit_shares <  bitasset_type::bit_usd.
   */
  price operator / ( const asset& a, const asset& b )
  {
    try 
    {
        price p;
        auto l = a; auto r = b;
        if( a.unit < b.unit ) 
        {
          std::swap(l,r);
          p.base_unit = a.unit;
          p.quote_unit = b.unit;
        }
        else
        {
          p.base_unit = b.unit;
          p.quote_unit = a.unit;
        }

        fc::bigint bl = l.amount;
        fc::bigint br = r.amount;
                            // bl   64.128 =>  0.64
        fc::bigint result = (bl <<= 64) / br;

        p.ratio = result;
       // ilog( "${a} / ${b} => ${r}", ("a",a)("b",b)("r",p) );
        return p;
    } FC_RETHROW_EXCEPTIONS( warn, "${a} / ${b}", ("a",a)("b",b) );
  }

  /**
   *  Assuming a.type is either the numerator.type or denominator.type in
   *  the price equation, return the number of the other asset type that
   *  could be exchanged at price p.
   *
   *  ie:  p = 3 usd/bts & a = 4 bts then result = 12 usd
   *  ie:  p = 3 usd/bts & a = 4 usd then result = 1.333 bts 
   */
  asset operator * ( const asset& a, const price& p )
  {
    try {
        if( a.unit == p.base_unit )
        {
            fc::bigint ba( a.amount ); // 64.64
            fc::bigint r( p.ratio ); // 64.64
            //fc::uint128 ba_test = ba; 

            auto amnt = ba * r; //  128.128
            amnt >>= 64; // 128.64 
            auto lg2 = amnt.log2();
            if( lg2 >= 128 )
            {
               FC_THROW_EXCEPTION( exception, "overflow ${a} * ${p}", ("a",a)("p",p) );
            }

            asset rtn;
            rtn.amount = amnt;
            rtn.unit = p.quote_unit;

            ilog( "${a} * ${p} => ${rtn}", ("a", a)("p",p )("rtn",rtn) );
            return rtn;
        }
        else if( a.unit == p.quote_unit )
        {
            fc::bigint amt( a.amount ); // 64.64
            amt <<= 64;  // 64.128
            fc::bigint pri( p.ratio ); // 64.64

            auto result = amt / pri;  // 64.64
            //auto test_result = result;
            //ilog( "test result: ${r}", ("r", std::string(test_result >>= 60) ) );
            auto lg2 = result.log2();
            if( lg2 >= 128 )
            {
             //  wlog( "." );
               FC_THROW_EXCEPTION( exception, 
                                    "overflow ${a} / ${p} = ${r} lg2 = ${l}", 
                                    ("a",a)("p",p)("r", std::string(result)  )("l",lg2) );
            }
            asset r;
            r.amount = result;
            r.unit   = p.base_unit;
            ilog( "${a} * ${p} => ${rtn}", ("a", a)("p",p )("rtn",r) );
            return r;
        }
        FC_THROW_EXCEPTION( exception, "type mismatch multiplying asset ${a} by price ${p}", 
                                            ("a",a)("p",p) );
    } FC_RETHROW_EXCEPTIONS( warn, "type mismatch multiplying asset ${a} by price ${p}", 
                                        ("a",a)("p",p) );

  }




} } // bts::blockchain
namespace fc
{
   void to_variant( const bts::blockchain::asset& var,  variant& vo )
   {
     vo = std::string(var);
   }
   void from_variant( const variant& var,  bts::blockchain::asset& vo )
   {
     vo = bts::blockchain::asset( var.as_string() );
   }
   void to_variant( const bts::blockchain::price& var,  variant& vo )
   {
     vo = std::string(var);
   }
   void from_variant( const variant& var,  bts::blockchain::price& vo )
   {
     vo = bts::blockchain::price( var.as_string() );
   }
}
