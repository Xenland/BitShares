#include <bts/bitname/bitname_fork_db.hpp>
#include <bts/db/level_pod_map.hpp>
#include <bts/difficulty.hpp>
#include <fc/reflect/variant.hpp>

#include <unordered_set>

#include <fc/log/logger.hpp>

struct fork_index
{
   fork_index():fork_difficulty(0){}
   fork_index( bts::bitname::name_id_type id, uint64_t fork_diff )
   :fork_difficulty(fork_diff),fork_header(id){}

   uint64_t     fork_difficulty;
   bts::bitname::name_id_type fork_header;
};

bool operator < ( const fork_index& a, const fork_index& b )
{
   return a.fork_difficulty          == b.fork_difficulty ? 
                   a.fork_header     <  b.fork_header : 
                   a.fork_difficulty <  b.fork_difficulty;
}
bool operator == ( const fork_index& a, const fork_index& b )
{
   return a.fork_difficulty  == b.fork_difficulty  && 
          a.fork_header      ==  b.fork_header; 
}

FC_REFLECT( fork_index, (fork_difficulty)(fork_header) );



namespace bts { namespace bitname {

  namespace detail 
  {
    class fork_db_impl 
    {
      public:
        db::level_pod_map<name_id_type,meta_header>                         _headers;
        db::level_pod_map<name_id_type,name_block>                          _blocks;

        /// note: invalid forks will have a difficulty of 0, best fork should have a highest difficulty
        /// TODO: switch to level_pod_set... no need to deal with the value...
        db::level_pod_map<fork_index,uint32_t>                              _forks; // index by difficulty
        db::level_pod_map<name_id_type, std::unordered_set<name_id_type> >  _nexts;
        db::level_pod_map<name_id_type,name_id_type>                        _unknown; // unknown id to the block that refs it.

        // cached for performance reasons... 

        void add_next( name_id_type prev, name_id_type next )
        {
           auto nexts = _nexts.fetch(prev);
           if( nexts.insert(next).second )
           {
             _nexts.store(prev,nexts);
           }
        }

        void update_fork( const meta_header& prev, const meta_header& next )
        {
            _forks.remove( fork_index(prev.id(),prev.chain_difficulty) );
            _forks.store( fork_index(next.id(),next.chain_difficulty), 0 );
        }

        /** calculate the difficulty, height, and valid state of every node after id */
        void update_chain( const name_id_type& update_id )
        {
            std::vector<name_id_type>  update_stack;
            update_stack.push_back(update_id);

            while( update_stack.size() )
            {
               auto cur_id = update_stack.back();
               update_stack.pop_back();

               auto cur_meta = _headers.fetch( cur_id );
               FC_ASSERT( cur_meta.height > 0 );

               auto itr = _nexts.find(cur_id);
               bool has_next = false;
               if( itr.valid() )
               {
                  auto next_set = itr.value();
                  for( auto itr = next_set.begin(); itr != next_set.end(); ++itr )
                  {
                     auto next_meta             = _headers.fetch( *itr );
                     next_meta.chain_difficulty = cur_meta.chain_difficulty + bts::difficulty( *itr ); 
                     next_meta.height           = cur_meta.height + 1;
                     next_meta.valid            = cur_meta.valid;
                     _headers.store( *itr, next_meta );
                     update_stack.push_back( *itr );
                  }
                  if( next_set.size() == 0 )
                  {
                    has_next = true;
                  }
               }
               if( !has_next )
               {
                 _forks.store( fork_index( cur_id, cur_meta.chain_difficulty), 0 );
               }
           }
        } // update_chain
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

  } FC_RETHROW_EXCEPTIONS( warn, "unable to open fork database ${path}", ("path",db_dir) ) }


  void fork_db::cache_header( const name_header& head )
  { try {
      auto id = head.id();
      meta_header meta(head);

      if( head.prev == name_id_type() ) // better be genesis!
      {
        // TODO: FC_ASSERT( id == genesis_id ) 
        meta.chain_difficulty = bts::difficulty(id);
        meta.height = 0;
        meta.valid  = true;
        my->_forks.store( fork_index( id, meta.chain_difficulty ), 0 );
        my->_headers.store(id,meta);
        return;
      }
      auto prev_meta_itr = my->_headers.find( head.prev );
      if( prev_meta_itr.valid() )
      {
         auto prev_meta = prev_meta_itr.value();
         if( prev_meta.height != -1 )
         {
             meta.height           = prev_meta.height + 1;
             meta.chain_difficulty = prev_meta.chain_difficulty + bts::difficulty(id);
             meta.valid            = prev_meta.valid;

             my->update_fork( prev_meta, meta );
         }
         my->add_next( prev_meta.id(), id );
      }
      else 
      {
         my->_unknown.store( head.prev, id );
      }
      my->_headers.store( id, meta );

      auto unknown_itr = my->_unknown.find(id);
      if( unknown_itr.valid() )
      {
          my->_unknown.remove( id );
          if( meta.height )
          {  // we just connected this chain back to genesis 
             my->update_chain( id );
          }
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

  meta_header fork_db::fetch_header( const name_id_type& id )
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
  { try {
    auto cur_meta = fetch_header(blk_id);
    FC_ASSERT( cur_meta.height > 0 ); // note: cannot set valid state on disconnected node!
    if( is_valid != cur_meta.valid )
    {
       cur_meta.valid = is_valid;
       my->_headers.store( blk_id, cur_meta );
       my->update_chain( blk_id );
       return;
    }
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

  name_id_type fork_db::best_fork_head_id()
  { try {
     fork_index last;
     my->_forks.last(last);
     return last.fork_header;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }

  name_id_type fork_db::best_fork_fetch_next( const name_id_type& b )
  { try {
     if( b == name_id_type() )
     {
        FC_ASSERT( !"TODO: return genesis id" );
     }
     FC_ASSERT( my->_headers.find(b).valid() );

     auto cur_id = best_fork_head_id();
     while( cur_id != name_id_type() )
     {
         auto cur_head = fetch_header(cur_id);
         if( cur_head.prev == b )
         {
           return cur_id;
         }
         cur_id = cur_head.prev;
     }
     FC_THROW_EXCEPTION( key_not_found_exception, "id ${x} is not in best fork", ("x",b) );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("b",b) ) }

  std::vector<meta_header> fork_db::get_forks()
  { try {
     std::vector<meta_header> result;
     auto itr = my->_forks.begin();
     while( itr.valid() )
     {
       result.push_back( fetch_header(itr.key().fork_header) );
       ++itr;
     }
     return result;
  } FC_RETHROW_EXCEPTIONS( warn, "" ) }



} }  // namespace bts::bitname
