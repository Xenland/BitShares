#pragma once


namespace bts { namespace dds {
  
  namespace detail { class transfer_output_cache_impl; }


  /**
   *  The transfer output cache tracks all unspent outputs that may 
   *  be spent with the signature of the owner.  This output cache
   *  must be divided into 1 MB chunks which are small enough that
   *  a 
   *
   */
  class transfer_output_cache
  {
     public:
       transfer_output_cache();
       ~transfer_output_cache();

       fc::sha224 calculate_hash()const;
       
     private:
       std::unique_ptr<detail::transfer_output_cache_impl> my;
  };


} } // bts::dds
