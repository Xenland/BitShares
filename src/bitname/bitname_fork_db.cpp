#include <bts/bitname/bitname_fork_db.hpp>
#include <bts/db/level_pod_map.hpp>
#include <bts/difficulty.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace bitname {

  namespace detail 
  {
    class fork_db_impl 
    {
      public:
        db::level_pod_map<name_id_type,name_header>                 _headers;
        db::level_pod_map<name_id_type,name_block>                  _blocks;
        db::level_pod_map<name_id_type,fork_state>                  _forks; // index by fork head
        db::level_pod_map<name_id_type, std::vector<name_id_type> > _nexts;
        db::level_pod_map<name_id_type,name_id_type>                _unknown; // unknown id to the block that refs it.

        name_id_type                                                _best_fork_head_id;

        void reverse_update( const name_id_type& start_id, uint64_t starting_diff = 0, uint32_t starting_depth = 0 )
        {
            auto start_head = _headers.fetch( start_id );
            auto fork_itr   = _forks.find( start_id );
            if( fork_itr.valid() )
            {
               auto fork_state = fork_itr.value();
               fork_state.fork_difficulty =  start_head.difficulty() + starting_diff;
               fork_state.height = starting_depth + 1;
               _forks.store( start_id, fork_state );
               return;
            }

            // TODO: update this method to be non recursive so that
            // we do not overflow the stack and crash for deep updates.
            auto cur_dif = start_head.difficulty() + starting_diff;
            auto next_ids_itr = _nexts.find( start_id );
            if( next_ids_itr.valid() )
            {
               auto next_ids = next_ids_itr.value();
               for( auto next_id_itr = next_ids.begin(); next_id_itr != next_ids.end(); ++next_id_itr )
               {
                 reverse_update( *next_id_itr, cur_dif, starting_depth + 1 );
               }
            }
        }

        void traverse_invalidate( const name_id_type& id )
        {
          // TODO: mark all forks derived from id as invalid
        }
    };

  } // namespace detail

  fork_db::fork_db()
  :my( new detail::fork_db_impl() )
  {}
 
  fork_db::~fork_db()
  {}

  void fork_db::open( const fc::path& db_dir, bool create )
  { try {
     if( create ) 
     {
        fc::create_directories( db_dir );
     }
     my->_headers.open( db_dir / "headers", create );
     my->_blocks.open( db_dir / "blocks", create );
     my->_forks.open( db_dir / "forks", create );
     my->_nexts.open( db_dir / "nexts", create );
     my->_unknown.open( db_dir / "unknown", create );

     fc::optional<fork_state> best_fork;
     auto forks = get_forks();
     for( auto f = forks.begin(); f != forks.end(); ++f )
     {
         if( !best_fork && !f->invalid ) 
         {
            best_fork = *f;
         }
         else if( best_fork )
         {
            if( !f->invalid && (f->fork_difficulty > best_fork->fork_difficulty) )
            {
              best_fork = *f;
            }
         }
     }
     if( best_fork )
     {
        my->_best_fork_head_id = best_fork->head_id;
     }

  } FC_RETHROW_EXCEPTIONS( warn, "unable to open fork database ${path}", ("path",db_dir) ) }


  void fork_db::cache_header( const name_header& head )
  { try {
      auto id = head.id();
      my->_headers.store( id, head );

      auto forks_itr = my->_forks.find( head.prev );
      if( forks_itr.valid() )
      {
         auto forkstate              = forks_itr.value();
         forkstate.head_id           = id;
         forkstate.fork_difficulty += head.difficulty();
         forkstate.height     += 1;
         my->_forks.remove( head.prev );
         my->_forks.store( id, forkstate );
      }
      else
      {
         // it doesn't extend a fork... is the previous
         // already in the unknown list?
         auto unknown_itr = my->_unknown.find(head.prev);
         if( !unknown_itr.valid() ) // then we are out of nowhere
         {
           my->_unknown.store( head.prev, id );
           fork_state new_fork;
           new_fork.fork_difficulty = bts::difficulty(id);
           new_fork.height = 1;
           new_fork.head_id = id;
           my->_forks.store( id, new_fork );
         }
      }

      /* link up the 'next pointers' */
      auto peers_itr = my->_nexts.find( head.prev );
      if( peers_itr.valid() ) 
      {
         auto peers = peers_itr.value();
         peers.push_back( id );
         my->_nexts.store( head.prev, peers );
      }
      else
      {
        std::vector<name_id_type> peers;
        peers.push_back(id);
        my->_nexts.store( head.prev, peers );
      }

      /** is the previous 'unknwon'?  if so add it */
      auto prev_itr = my->_headers.find( head.prev );
      if( !peers_itr.valid() )
      {
         my->_unknown.store( head.prev, id );
      }
      
      /** is this header currently 'unknown', if so
       *  then we just grew the chain and it probably
       *  needs updated.
       */
      auto unknown_itr = my->_unknown.find( id );
      if( unknown_itr.valid() )
      {
        my->_unknown.remove( id );
        my->reverse_update( id );
        // ... what does this mean for the chain ?
      }

  } FC_RETHROW_EXCEPTIONS( warn, "", ("header",head) ) }

  void fork_db::cache_block( const name_block& b )
  {
      cache_header( b );
      my->_blocks.store( b.id(), b );
  }

  std::vector<name_id_type> fork_db::fetch_unknown()
  {
     std::vector<name_id_type> result;
     auto itr = my->_unknown.begin();
     while( itr.valid() )
     {
       result.push_back( itr.value() );
       ++itr;
     }
     return result;
  }

  name_header fork_db::fetch_header( const name_id_type& id )
  { try {
     return my->_headers.fetch(id);
  } FC_RETHROW_EXCEPTIONS( warn, "", ("id",id) ) }

  fc::optional<name_block>  fork_db::fetch_block( const name_id_type& id )
  { try {
     auto head = fetch_header( id );
     if( head.trxs_hash == name_trxs_hash_type() )
     {
       return name_block(head);
     }

     try {
        // TODO: verify that _blocks.fetch() throws key_not_found exception
        // if no block is known for id.
        return my->_blocks.fetch(id);
     } 
     catch ( const fc::key_not_found_exception& e )
     {
       return fc::optional<name_block>();
     }
  } FC_RETHROW_EXCEPTIONS( warn, "", ("id",id) ) }

  void fork_db::set_valid( const name_id_type& blk_id, bool is_valid )
  {
    if( is_valid == false )
    {
      my->traverse_invalidate( blk_id );
    }
    else // only the fork head can me marked valid... 
    {

      auto fork_val = my->_forks.fetch( blk_id );
      fork_val.invalid = false;
      my->_forks.store( blk_id, fork_val );
    }
  }

  name_id_type fork_db::best_fork_head_id()const
  {
     return my->_best_fork_head_id;
  }

  name_id_type fork_db::best_fork_fetch_next( const name_id_type& b )const
  {
      // TODO: implment 
     return name_id_type();  
  }

  std::vector<fork_state> fork_db::get_forks()const
  { try {
     std::vector<fork_state> result;
     auto itr = my->_forks.begin();
     while( itr.valid() )
     {
       result.push_back( itr.value() );
       ++itr;
     }
     return result;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }



} }  // namespace bts::bitname
