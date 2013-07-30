#include <bts/merkle_tree.hpp>
#include <fc/exception/exception.hpp>

namespace bts {

  uint160 merkle_branch::calculate_root()const
  {
     if( mid_states.size() == 0 ) return uint160();
     if( mid_states.size() == 1 ) return mid_states[0];
     FC_ASSERT( !"TODO: Merged Mining is Not Yet Implemented" );
  }

} // namespace bts
