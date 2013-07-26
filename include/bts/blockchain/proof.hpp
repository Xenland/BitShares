#pragma once
#include <fc/crypto/sha224.hpp>
#include <fc/reflect/reflect.hpp>
#include <bts/merkel_tree.hpp>

namespace bts
{

  /**
   *  @brief Data that is paired with 
   */
  struct proof
  {
     proof()
     :branch_path(-1),nonce(0){}

     /**
      *  Does not include the leaf_hash which must be passed to the
      *  merkel_root() method to properly reconstruct the merkel root.
      *
      *  The leaf is not included in this struct because it can always be
      *  calculated from the header and it is helpful to avoid an extra
      *  28 bytes.
      *
      *  Includes the full merkel branch to the hash of the header.
      */
     merkel_branch           header_branch;
     uint64_t                nonce;
  };

}

FC_REFLECT( bts::proof, (header_branch)(nonce) )
