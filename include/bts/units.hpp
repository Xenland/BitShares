#pragma once
#include <fc/reflect/reflect.hpp>

namespace bts 
{

/**
 *  All balanes are annotated with a unit, each BitShare 
 *  chain supports 8 basic units
 */
enum unit_type 
{
    BitShares  = 0,
    BitGold    = 1,
    BitSilver  = 2,
    BitUSD     = 3,
    BitEUR     = 4,
    BitYUAN    = 5,
    BitGBP     = 6,
    BitBTC     = 7,
    NumUnits    
};

struct bond_type
{
   bond_type():issue_type(BitShares),backing_type(BitShares){}
   unit_type  issue_type;
   unit_type  backing_type;
};

} // namespace bts

FC_REFLECT_ENUM( bts::unit_type, 
      (BitShares)
      (BitGold)
      (BitSilver)
      (BitUSD)
      (BitEUR)
      (BitYUAN)
      (BitGBP)
      (BitBTC)
      (NumUnits) 
)

FC_REFLECT( bts::bond_type, (issue_type)(backing_type) )

// TODO: define raw pack to 1 byte

