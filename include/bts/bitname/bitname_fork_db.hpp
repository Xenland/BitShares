#pragma once
#include <bts/bitname/bitname_fork_db.hpp>
#include <bts/bitname/bitname_block.hpp>
#include <fc/filesystem.hpp>

namespace bts { namespace bitname {

  namespace detail { class fork_db_impl; }

  struct meta_header : public name_header
  {
     meta_header( const name_header& h )
     :name_header(h),chain_difficulty(0),height(-1),valid(true),unavailable_count(0){}
     meta_header()
     :chain_difficulty(0),height(-1),valid(true){}

     uint64_t chain_difficulty;
     int32_t  height;
     bool     valid;
     uint32_t unavailable_count;
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
       std::vector<name_id_type> best_fork_ids();

       name_id_type              best_fork_head_id();
       uint32_t                  best_fork_height();
       name_id_type              best_fork_fetch_next( const name_id_type& b );

       /** 
        *  Return the header at hieight H in the best fork.
        */
       meta_header              best_fork_fetch_at( uint32_t height );

       /**
        * All forks that branch from a particular node.
        */
       std::vector<name_id_type> fetch_next( const name_id_type& b );
       meta_header               fetch_header( const name_id_type& b );
       fc::optional<name_block>  fetch_block( const name_id_type& b );

       /**
        *  Marks all forks derived from this block invalid.
        */
       void set_valid( const name_id_type& blk_id, bool is_valid );

       /**
        *  @return the list of potential forks sorted by difficulty.  The
        *  most difficulty fork may not be 'valid'.
        */
       std::vector<meta_header> get_forks(); 
    
     private:
       std::unique_ptr<detail::fork_db_impl> my;
  };


} } // bts::bitname

FC_REFLECT_DERIVED( bts::bitname::meta_header, (bts::bitname::name_header), (chain_difficulty)(height)(valid) )
