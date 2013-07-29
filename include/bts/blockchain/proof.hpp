#pragma once
#include <fc/reflect/reflect.hpp>
#include <bts/merkle_tree.hpp>

namespace bts { namespace blockchain {

  /**
   *  @brief Data that is paired with 
   */
  struct proof
  {
     proof()
     :branch_path(),nonce(0){}

     /**
      *  Does not include the leaf_hash which must be passed to the
      *  merkle_root() method to properly reconstruct the merkle root.
      *
      *  The leaf is not included in this struct because it can always be
      *  calculated from the header and it is helpful to avoid an extra
      *  28 bytes.
      *
      *  Includes the full merkle branch to the hash of the header.
      */
     merkle_branch           branch_path;
     uint32_t                nonce;
  };

} } // bts::blockchain

FC_REFLECT( bts::blockchain::proof, (branch_path)(nonce) )
