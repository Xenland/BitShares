#include <bts/blockchain/blockchain_market_db.hpp>
#include <bts/db/level_pod_map.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace blockchain {

  namespace detail
  {
     class market_db_impl
     {
        public:
           db::level_pod_map<market_order,uint32_t> _bids;
           db::level_pod_map<market_order,uint32_t> _asks;
     };

  } // namespace detail
  market_order::market_order( const price& p, const output_reference& loc )
  :base_unit(p.base_unit),quote_unit(p.quote_unit),ratio( p.ratio ),location(loc)
  {}

  price market_order::get_price()const
  {
     return price( ratio, base_unit, quote_unit );
  }


  bool operator == ( const market_order& a, const market_order& b )
  {
     return a.ratio == b.ratio &&
            a.location == b.location &&
            a.base_unit == b.base_unit &&
            a.quote_unit == b.quote_unit;
  }
  bool operator < ( const market_order& a, const market_order& b )
  {
     if( a.base_unit.value < b.base_unit.value ) return true;
     if( a.base_unit.value > b.base_unit.value ) return false;
     if( a.quote_unit.value < b.quote_unit.value ) return true;
     if( a.quote_unit.value > b.quote_unit ) return false;
     if( a.ratio < b.ratio ) return true;
     if( a.ratio > b.ratio ) return false;
     return a.location < b.location;
  }

  market_db::market_db()
  :my( new detail::market_db_impl() )
  {
  }

  market_db::~market_db()
  {}

  void market_db::open( const fc::path& db_dir )
  { try {

     fc::create_directories( db_dir / "bids" );
     fc::create_directories( db_dir / "asks" );

     my->_bids.open( db_dir / "bids" );
     my->_asks.open( db_dir / "asks" );

  } FC_RETHROW_EXCEPTIONS( warn, "unable to open market db ${dir}", ("dir",db_dir) ) }

  void market_db::insert_bid( const market_order& m )
  {
     my->_bids.store( m, 0 );
  }
  void market_db::insert_ask( const market_order& m )
  {
     my->_asks.store( m, 0 );
  }
  void market_db::remove_bid( const market_order& m )
  {
     my->_bids.remove(m);
  }
  void market_db::remove_ask( const market_order& m )
  {
     my->_asks.remove(m);
  }

  /** @pre quote > base  */
  fc::optional<market_order> market_db::get_highest_bid( asset::type quote, asset::type base )
  {
    FC_ASSERT( quote > base );
    fc::optional<market_order> highest_bid;

    return highest_bid;
  }
  /** @pre quote > base  */
  fc::optional<market_order> market_db::get_lowest_ask( asset::type quote, asset::type base )
  {
    FC_ASSERT( quote > base );
    fc::optional<market_order> lowest_ask;

    return lowest_ask;
  }


} } // bts::blockchain
