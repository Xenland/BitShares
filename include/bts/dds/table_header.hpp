#pragma once
#include <fc/reflect/reflect.hpp>
#include <fc/crypto/sha224.hpp>
#include <set>

namespace bts
{
  /**
   *  @brief Captures table header information for DDS tables.
   */
  class table_header
  {
     public:
       table_header()
       :table_type( null_claim_type ),
        size(0){}

       claim_type              table_type;    
       std::vector<fc::sha224> chunk_hashes; ///< The hashes of merkel root of each chunk
       std::set<uint32_t>      free_list;    ///< Table entries that are unused, stored in-order
       uint32_t                size;         ///< The last element in the table
  };
}

FC_REFLECT( bts::table_header, (table_type)(chunk_hashes)(free_list)(size) )
