#include <bts/dds/dividend_cache.hpp>
#include <fc/interprocess/file_mapping.hpp>

namespace bts { namespace dds {
  #define BLOCKS_PER_YEAR     (12*24*365)
  #define EXPECTED_FILE_SIZE  ((BLOCKS_PER_YEAR+1)*sizeof(uint64_t))
  #define HEAD_BLOCK_NUMBER   (0)
  
  namespace detail { 

    class dividend_cache_impl
    {
       public:
          dividend_cache_impl()
          :_data(nullptr){}

          std::unique_ptr<fc::file_mapping>    _filemap;
          std::unique_ptr<fc::mapped_region>   _maped_region;

          /**
           *  File format is the first 8 bytes contains the block number,
           *  the next BLOCKS_PER_YEAR entries are a circular buffer.
           */
          uint64_t*                            _data;

          void map_file( const fc::path& )
          {
          }

          void initialize( const fc::path& )
          {
          }

          uint64_t get_dividend_percent( uint64_t block_num )
          {

          }
    };

  } // namespace detail

  dividend_cache::dividend_cache()
  :my( new detail::dividend_cache_impl() )
  {
  }

  dividend_cache::~dividend_cache()
  {
    try
    {
        sync();
    } 
    catch ( fc::exception& e )
    {
       // TODO: log the exception
    }
  }

  void dividend_cache::load( const fc::path& cache_file )
  {
      my->_filepath = cache_file;
      if( !fc::exists( cache_file )
      {
         // TODO: conditionally log init
         my->initialize( cache_file );
         return;
      }
      if( fc::file_size( cache_file ) != EXPECTED_FILE_SIZE )
      {
         // TODO: conditionally log warning and re-init
         my->initialize( cache_file );
         return;
      }
      my->map_file(cache_file);
  }

  uint64_t dividend_cache::head_block_number()const
  {
     FC_ASSERT( my->_data != nullptr );
     return _data[HEAD_BLOCK_NUMBER];
  }


  uint64_t dividend_cache::calculate_dividends( uint64_t start_block, uint64_t end_block )const
  {
    uint64_t head = head_block_number();

    FC_ASSERT( start_block > end_block )
    FC_ASSERT( end_block == uint64_t(-1) || end_block <= head )

    if( end_block == uint64_t(-1) /*max value*/  )
    {
      end_block = head;
    }
    if( end_block == head )
    {
      return my->get_dividend_percent( start_block );
    }
    return my->get_dividend_percent( start_block ) - my->get_dividend_percent(end_block+1);
  }


  uint64_t dividend_cache::push_back( uint64_t dividend_percent )
  {
    uint64_t tail_value = my->get_oldest_dividend();

    uint64_t*       pos = _data + 1;
    const uint64_t* end = pos + BLOCKS_PER_YEAR;

    while( pos != end )
    {
       *pos += dividend_percent; 
       ++pos;
    }

    // increment the block index
    _data[HEAD_BLOCK_NUMBER]++;

    return tail_value;
  }


  uint64_t dividend_cache::push_front( uint64_t dividend_percent )
  {
    FC_ASSERT( false ); // TODO implement this method 
  }

  fc::sha224 dividend_cache::calculate_hash()const
  {
    FC_ASSERT( my->_data != nullptr );
    return fc::sha224::hash( (char*)my->_data, EXPECTED_FILE_SIZE );
  }




}} // namespace bts::dds
