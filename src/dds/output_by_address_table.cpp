#include <bts/config.hpp>
#include <bts/dds/output_by_address_table.hpp>
#include <bts/dds/table_header.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/interprocess/mmap_struct.hpp>
#include <fc/exception/exception.hpp>
#include <fc/io/fstream.hpp>
#include <fc/io/raw.hpp>
#include <algorithm>

#include <unordered_map>

#define OUTPUTS_PER_CHUNK  uint64_t(2*1024*1024/sizeof(bts::output_by_address_table::entry))


namespace bts 
{

  typedef fc::mmap_struct< fc::array<output_by_address_table::entry, OUTPUTS_PER_CHUNK > >  mmap_entry_chunk;
  typedef std::unique_ptr<mmap_entry_chunk>                                                 mmap_entry_chunk_ptr;

  namespace detail 
  {
      class output_by_address_table_impl
      {
        public:
          output_by_address_table_impl()
          :dirty(true)
          {
            outputs_by_block.resize( BLOCKS_PER_YEAR );
          }

          /// the directory where all data files are maintained for this table.
          fc::path                           table_dir;

          /**
           *  A vector of vectors that stores the output indexes for
           *  each block going back 1 year or BLOCKS_PER_YEAR blocks.
           *
           *  TODO: find a way to save / load this to accelerate startup
           */
          std::vector< std::vector<uint32_t> > outputs_by_block;

          /**
           *  Maps an output to its index in the table, recalculated 
           *  on load.
           *
           *  TODO: find a way to save / load this to accelerate startup
           */
          std::unordered_map<output_reference, uint32_t> output_index;

          std::vector<mmap_entry_chunk_ptr>  entries;
          std::vector<bool>                  dirty_chunks;
          bool                               dirty;

          // header data
          fc::sha224                         digest_hash;
          table_header                       table_head;

          void update_hashes()
          {
             if( dirty )
             {
               for( uint32_t i = 0; i < dirty_chunks.size(); ++i )
               {
                  if( dirty_chunks[i] )
                  {
                     table_head.chunk_hashes[i] = fc::sha224::hash( (char*) (*entries[i])->data, 
                                                                     sizeof( (*entries[i])->data ) );
                     dirty_chunks[i] = 0;
                  }
               }
               fc::sha224::encoder enc;
               fc::raw::pack( enc, table_head );
               digest_hash = enc.result();
             }
          }

          /**
           *   For every entry in the given chunk, remove it from the output_index
           */
          void clear_chunk_index( uint32_t chunk_id )
          {
              FC_ASSERT( !!entries[chunk_id] );
              mmap_entry_chunk& ec = *entries[chunk_id];
              for( uint32_t i = 0; i < OUTPUTS_PER_CHUNK; ++i )
              {
                 output_index.erase( ec->at( i ).out_id );
              }
          }

          /** maintains an ordered list of indexes for a particular block,
           *  in theory, other than adding a new block, this list should only
           *  be subtracted from in the future.
           */
          void set_block_index( uint32_t block_num, uint32_t idx )
          {
             auto itr = std::lower_bound( outputs_by_block[block_num].begin(), 
                                          outputs_by_block[block_num].end(), idx );
             if( itr != outputs_by_block[block_num].end() && *itr == idx)
             {
                FC_THROW_EXCEPTION( assert_exception, "Index ${idx} already stored for block ${block}",
                                    ("idx",idx)("block",block_num) );
             }
             outputs_by_block[block_num].insert( itr, idx );
          }

          void clear_block_index( uint32_t block_num, uint32_t idx )
          {
             auto itr = std::lower_bound( outputs_by_block[block_num].begin(), 
                                          outputs_by_block[block_num].end(), idx );
             if( itr == outputs_by_block[block_num].end() )
             {
                FC_THROW_EXCEPTION( assert_exception, 
                        "Block number ${num} does not contain an output at index ${idx}",
                        ("num",block_num)("idx",idx) );

             }
             outputs_by_block[block_num].erase(itr);
          }
          /**
           *   Performs a check for redundancy before setting p
           */
          void set_index( const output_reference& r, uint32_t p )
          {
              ilog( "set index ${r} = ${p}", ("r",r)("p",p) );
              FC_ASSERT( output_index.find(r) == output_index.end() );
              output_index[r] = p;
          }

          /**
           *  Checks that the output reference r is stored in the index
           *  and that it is found at idx, then clears it.
           */
          void clear_index( const output_reference& r, uint32_t idx )
          {
              ilog( "clear index ${r} = ${p}", ("r",r)("p",idx) );
              FC_ASSERT( output_index.find(r) != output_index.end() );
              FC_ASSERT( output_index[r] == idx );
              output_index.erase( r );
          }

          /**
           *  For every entry in the given chunk, add it to the output_index
           */
          void index_chunk( uint32_t chunk_id )
          {
              FC_ASSERT( !!entries[chunk_id] );
              mmap_entry_chunk& ec = *entries[chunk_id];
              for( uint32_t i = 0; i < OUTPUTS_PER_CHUNK; ++i )
              {
                 if( ec->at(i).out_id != output_reference() )
                 {
                     set_index( ec->at(i).out_id, OUTPUTS_PER_CHUNK*chunk_id + i ); 
                     set_block_index( ec->at(i).block_num, OUTPUTS_PER_CHUNK*chunk_id + i ); 
                 }
              }
          }

          void load_header( const fc::path& p, bool create  )
          {
             try 
             {
                 if( !fc::exists( p ) )
                 {
                    if( !create ) 
                    {
                        FC_THROW_EXCEPTION( file_not_found_exception, 
                                  "Unable to find header '${header}' for "
                                  "output_by_address_table", ("header", p ) );
                    }
                    table_head.table_type = claim_by_address;
                    return;
                 }

                 fc::ifstream in( p, fc::ifstream::binary );
                 fc::raw::unpack( in, table_head );

             }FC_RETHROW_EXCEPTIONS( warn, "Unable to load header '${header}' for "
                                     "output_by_address_table", ("header", p ) )
          }

          void init_chunk( uint32_t chunk_id )
          {
             if( chunk_id >= entries.size() )
             {
               ilog( "resize.." );
               entries.resize( chunk_id + 1 );
               dirty_chunks.resize( chunk_id + 1);
               table_head.chunk_hashes.resize( chunk_id + 1);
             }

             if( chunk_id == table_head.size )
             {
               ++table_head.size;
             }

             if( !entries[chunk_id] )
             {
                entries[chunk_id].reset( new mmap_entry_chunk() );  
                fc::path chunk_file = table_dir / fc::format_string( "chunk${i}.dat", 
                                                      fc::variant_object("i",chunk_id) );
                entries[chunk_id]->open( chunk_file, true/*create*/ );
                index_chunk( chunk_id );
             }
          }


          void validate_index_range( uint32_t index )
          {
             if( index > table_head.size )
             {
                FC_THROW_EXCEPTION( out_of_range_exception, 
                    "Index ${i} is larger than table size", ("i",index) );
             }
          }
      };

  } // namespace detail 

  output_by_address_table::entry::entry()
  {
     memset( (char*)this, 0, sizeof(*this) );
  }


  output_by_address_table::output_by_address_table()
  :my( new detail::output_by_address_table_impl() )
  {
  }

  output_by_address_table::~output_by_address_table()
  {
      ilog( "" );
      save();
  }

  /**
   *  Saves the table state to disk
   */
  void output_by_address_table::save()
  {
      ilog( "" );
      for( uint32_t i = 0; i < my->entries.size(); ++i )
      {
         if( my->entries[i] )
         {
          ilog( "flush! ${i}", ("i",i) );
           my->entries[i]->flush();
           ilog( ".." );
         }
         else 
         {
          wlog( "what? why is entries[${i}] null?", ("i",i) );
         }
      }
      my->update_hashes();

      fc::ofstream out( my->table_dir / "header.dat" );
      fc::raw::pack( out, my->table_head );
  }

  /**
   *    
   *
   */
  void output_by_address_table::load( const fc::path& table_dir, bool create )
  {
     if( !fc::exists( table_dir ) )
     {
        if( !create )
        {
            FC_THROW_EXCEPTION( file_not_found_exception, "directory '${dir}' does not exist", ("dir", table_dir) );
        }
        else
        {
            fc::create_directories( table_dir );
        }
     }
     my->table_dir = table_dir;
     my->load_header( table_dir / "header.dat", create );

     my->entries.resize( my->table_head.chunk_hashes.size() );
     for( uint32_t i = 0; i < my->table_head.chunk_hashes.size(); ++i )
     {
         my->init_chunk( 0 );
     }
  }


  fc::sha224 output_by_address_table::calculate_hash()const
  {
     my->update_hashes();
     return my->digest_hash;
  }


  const table_header&     output_by_address_table::get_header()const
  {
      my->update_hashes();
      return my->table_head;
  }

  std::vector<char>       output_by_address_table::get_chunk( uint32_t chunk_num )const
  {
      if( chunk_num < my->entries.size() &&  !!my->entries[chunk_num] )
      {
          size_t s = (const char*)(*my->entries[chunk_num])->end() - (char*)(*my->entries[chunk_num])->begin(); 
          std::vector<char>  ch(s);
          memcpy(ch.data(),  (const char*)(*my->entries[chunk_num])->begin(), s );
          return ch;
      }
      FC_THROW_EXCEPTION( assert_exception,  "chunk_num: ${c} is out of range or chunk ${c} was null", ("c",chunk_num));
  }

  /**
   *  This method replaces the current chunk at chunk_num with data in ch.  It must clear the
   *  output index of all entries in the old chunk and re-index the new chunk.  Lastly it
   *  must mark the chunk as dirty.
   */
  void  output_by_address_table::set_chunk( uint32_t chunk_num, const std::vector<char>& ch )
  {
      if( chunk_num < my->entries.size() )
      {
          if( !my->entries[chunk_num] )
          {
             my->init_chunk( chunk_num );
          }
          my->clear_chunk_index( chunk_num );

          size_t s = (const char*)(*my->entries[chunk_num])->end() - (char*)(*my->entries[chunk_num])->begin(); 
          memcpy((char*)(*my->entries[chunk_num])->begin(), ch.data(), s );

          my->index_chunk( chunk_num );
      }
      FC_THROW_EXCEPTION( out_of_range_exception, 
                          "chunk ${chunk_num} is out of range [0,${max})", 
                          ("chunk_num", chunk_num)
                          ("max",my->entries.size() ) );
  }

  /**
   *  @return the set of unspent outputs for a particular block.
   */
  std::vector<uint32_t> output_by_address_table::get_unspent_by_block( uint32_t block_number )
  {
     return my->outputs_by_block[block_number%BLOCKS_PER_YEAR];
  }


  /**
   *  Stores entry e in the table and returns the index it was
   *  stored at.
   *
   *  @param index - the location where e should be stored
   *
   *  @pre  index is in the free list and the contents are zeroed || index == size
   *  @pre  index <= table_header.size
   *  @post index is no longer in the free_list
   */
  void  output_by_address_table::store( uint32_t index, const entry& e )
  {
     uint32_t chunk_id  = index / OUTPUTS_PER_CHUNK;
     uint32_t chunk_idx = index % OUTPUTS_PER_CHUNK;
     ilog( "chunk_id ${i}.${d}  index: ${idx}", ("i", chunk_id)("d",chunk_idx)("idx",index) );

     my->validate_index_range(index);

     if( index != my->table_head.size && 
         my->table_head.free_list.find( index ) == my->table_head.free_list.end() )
     {
        FC_THROW_EXCEPTION( assert_exception, "index ${i} is already in use, freelist ${l}", ("i", index)("l",my->table_head.free_list) );
     }
     my->init_chunk( chunk_id );
     
     //ilog( "chunk idx: ${i} \n${old}\n${empty}", ("i", index )("old", (*my->entries[chunk_id])->at(chunk_idx))("empty",entry()) );
     // check to make sure that the data is null
     FC_ASSERT( (*my->entries[chunk_id])->at( chunk_idx ) == entry() );

     my->set_index( e.out_id, index );
     wlog( "set_block_index ${b} += ${i}", ("b",e.block_num)("i",index) );
     my->set_block_index( e.block_num, index );
     (*my->entries[chunk_id])->at( chunk_idx ) = e;

     my->table_head.free_list.erase(index);
     my->dirty_chunks[chunk_id] = true;
  }

  uint32_t output_by_address_table::get_free_slot()
  {
     if( my->table_head.free_list.size() > 0 )
     {
       return *my->table_head.free_list.begin();
     }
     return my->table_head.size;
  }

  /**
   *  Releases the specified index and places it in the 
   *  free-list.
   *
   *  @pre index is not in the free list
   *  @post index is in the free list and the contents of location at index are zeroed.
   *  @post index is not found output_index
   *
   *  @throw if index has already been 'cleared'
   */
  void     output_by_address_table::clear( uint32_t index )
  {
      my->validate_index_range(index);
      if( !my->table_head.free_list.insert( index ).second )
      {
        FC_THROW_EXCEPTION( assert_exception, "Index ${i} is already in the free list.", ("i",index) );
      }
      if( index >= my->table_head.size )
      {
        FC_THROW_EXCEPTION( out_of_range_exception, 
              "Index ${i} is greater than the output_by_address table's size ${s}.", 
                           ("i",index)
                           ("s",my->table_head.size) );
      }
      uint32_t chunk_id  = index / OUTPUTS_PER_CHUNK;
      uint32_t chunk_idx = index % OUTPUTS_PER_CHUNK;

      FC_ASSERT( !!my->entries[chunk_id] );
      entry& e = (*my->entries[chunk_id])->at( chunk_idx );

      wlog( "clear_block_index block ${b} += ${i}", ("b",e.block_num)("i",index) );
      my->clear_block_index( e.block_num, index );
      ilog( "clear ${i}", ("i",index) );
      my->clear_index( get(index).out_id, index );

      e = entry();
      assert( (*my->entries[chunk_id])->at( chunk_idx ) == entry() );
      ilog( "done clear" );
      my->dirty_chunks[chunk_id] = true;
      my->dirty = true;
  }


  /**
   *  Finds the output and returns its index, throws if the
   *  output was not found.
   *
   *  @throws key_not_found_exception if out was not in the table.
   */
  uint32_t output_by_address_table::find( const output_reference& out )
  {
     auto itr = my->output_index.find(out);
     if( itr == my->output_index.end() )
     {
        FC_THROW_EXCEPTION( key_not_found_exception, "Unable to find output ${out}", ("out",out) );
     }
     return itr->second;
  }

  const output_by_address_table::entry&   output_by_address_table::get( uint32_t index )
  {
     my->validate_index_range(index);
     uint32_t chunk_id  = index / OUTPUTS_PER_CHUNK;
     uint32_t chunk_idx = index % OUTPUTS_PER_CHUNK;
     return (*my->entries[chunk_id])->at( chunk_idx );
  }


} // namespace bts
