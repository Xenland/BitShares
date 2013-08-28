#pragma once
#include <bts/bitname/bitname_fork_db.hpp>
#include <bts/bitname/bitname_block.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace bitname {

  namespace detail { class fork_db_impl; }

  struct fork_state
  {
    fork_state()
    :fork_difficulty(0),invalid(false),height(0){}

    uint64_t       fork_difficulty; ///< cumulative difficulty of all headers back to origin
    bool           invalid;         ///< just because it isn't marked invalid, doesn't mean it is valid  
    uint32_t       height;          ///< height of the fork
    name_id_type   head_id;
  };

  /**
   *  Tracks the state of all headers that we learn about and organizes
   *  them into the various forks and tracks the validity of forks.
   *
   */
  class fork_db 
  {
     public:
       fork_db();
       ~fork_db();

       void open( const fc::path& db_dir, bool create );

       void cache_header( const name_header& head );
       void cache_block( const name_block& blk );

       /**
        *  All header IDs that we do not know the header for.  
        */
       std::vector<name_id_type> fetch_unknown();

       name_id_type              best_fork_head_id()const;
       name_id_type              best_fork_fetch_next( const name_id_type& b )const;

       /**
        * All forks that branch from a particular node.
        */
       std::vector<name_id_type> fetch_next( const name_id_type& b );
       name_header               fetch_header( const name_id_type& b );
       fc::optional<name_block>  fetch_block( const name_id_type& b );

       /**
        *  Marks all forks derived from this block invalid.
        */
       void set_valid( const name_id_type& blk_id, bool is_valid );

       /**
        *  @return the list of potential forks sorted by difficulty.  The
        *  most difficulty fork may not be 'valid'.
        */
       std::vector<fork_state> get_forks()const; 
    
     private:
       std::unique_ptr<detail::fork_db_impl> my;
  };


} } // bts::bitname

FC_REFLECT( bts::bitname::fork_state, (fork_difficulty)(invalid)(height)(head_id) )
