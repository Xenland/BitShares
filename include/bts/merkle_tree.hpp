#pragma once
#include <fc/crypto/sha224.hpp>
#include <vector>

namespace bts {

  /**
   *  Provides a merkle branch that proves a hash was
   *  included in the root.
   */
  struct merkle_branch
  {
     merkle_branch()
     :branch(0){}

     fc::sha224 calculate_root()const;

     uint32_t                branch;
     std::vector<fc::sha224> mid_states;
  };

  /**
   *  Maintains a merkle tree as updates are made via
   *  get/set.
   */
  struct merkle_tree
  {
       void     resize( uint64_t s );
       uint64_t size();

       /** 
        *  Updates all hashes in the merkle branch to index.
        *  @throw out_of_range_exception if index > size
        */
       void       set( uint64_t index, const fc::sha224& val );

       /**
        *  @throw out_of_range_exception if index > size
        */
       fc::sha224 get( uint32_t index );

       /**
        *  @return the full merkle branch for the tree.
        */
       merkle_branch get_branch( uint32_t index )const;
    
       /**
        *  @note do not modify this field directly, it will
        *        automatically be updated after every call to
        *        set / get.
        */
       fc::sha224                                mroot;

       /**
        *  mtree[0] is the leef layer of the tree
        *  mtree[mtree.size()-1].size() should always be 2
        *
        *  @note only modify this via get/set to keep the
        *        struture accurate.  This is public for
        *        serialization purposes only.  
        *
        *        TODO: update fc::reflect to support private
        *              members.
        */
       std::vector< std::vector < fc::sha224 > > mtree;
  };

} 
#include <fc/reflect/reflect.hpp>
FC_REFLECT( bts::merkle_branch, (branch)(mid_states) )
FC_REFLECT( bts::merkle_tree, (mroot)(mtree) )

