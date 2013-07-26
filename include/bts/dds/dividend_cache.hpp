#pragma once
#include <fc/filesystem/path.hpp>
#include <fc/crypto/sha224.hpp>
#include <memory>

namespace bts { namespace dds {
  
  namespace detail { class dividend_cache_impl; }

  /**
   *  Maintains the total number of dividends earned per block for the past year in
   *  a memory mapped file.  A memory mapped file is used for performance / persistance
   *  reasons.
   *
   *  There is 1 block every 5 minutes, 12 per hour, 288 per day 
   *  or 105,120 per year resulting in a file that contains about 822KB. 
   *
   *  Dividends are stored via an accumulator to make it easy to calculate the total
   *  number of dividends paid over any given period of time.  Dividends are stored as
   *  a percent of the total money supply * 1,000,000 for precision. 
   *
   *  Dividends are stored in a circular buffer that rotates as new blocks are added
   *  and removed.
   *
   *  @note all percents are represented as fixed point integerges with 9
   *        digits of precision. 
   */
  class dividend_cache
  {
    public:
      dividend_cache();
      ~dividend_cache();
      
      /**
       *  Maps cache_file to memory.  If it does not exist, then it will
       *  create the file and initialize the contents to 0.
       */
      void load( const fc::path& cache_file );

      /**
       *  Sync's the cache to disk.
       */
      void sync();

      /**
       *  Calculates the hash of the dividend cache (if dirty) otherwise returns
       *  the last calculated hash.   The hash is considered dirty anytime push_back()
       *  or push_front() is called.   
       */
      fc::sha224 calculate_hash()const;

      /**
       *  @return the most recent block number.
       */
      uint64_t head_block_number()const;

      /**
       *   @param end_block - may be -1 or any value <= head_block_number()
       *
       *   @return the  percent of the total money supply payable as
       *   dividends between start and end block inclusive.  
       *
       *   @pre start_block <= end_block
       *
       *   @throw if start_block is older than 1 year or endblock > head_block_number
       */
      uint64_t calculate_dividends( uint64_t start_block, uint64_t end_block )const;

      /** 
       *  @param dividend_percent the percent of dividends paid on the newest block
       *  @return the percent of dividends paid on the oldest block 
       *
       */
      uint64_t push_back( uint64_t dividend_percent );

      /**
       *  Subtracts the newest value from all blocks and then
       *  replaces the newest value with dividend_percent.
       */
      uint64_t push_front( uint64_t dividend_percent );

    private:
      std::unique_ptr<detail::dividend_cache_impl> my;
  };


} } // bts::dds
