#include <bts/dds/deterministic_data_store.hpp>
#include <bts/dds/output_by_address_table.hpp>
#include <fc/io/raw.hpp>

namespace bts { namespace dds {
  
  namespace detail 
  { 
    class deterministic_data_store_impl
    {
       public:
          fc::path                dds_dir;
          output_by_address_table out_by_addr_tbl;
          dds_index               ddsindex;
    };
  } // namespace detail


  deterministic_data_store::deterministic_data_store()
  :my( new detail::deterministic_data_store_impl() )
  {
  }

  deterministic_data_store::~deterministic_data_store()
  {
  }

  /**
   *  Loads data from the specified directory or creates the directory if it does
   *  not exist.
   */
  void deterministic_data_store::load( const fc::path& data_store_dir, bool create )
  {
     my->dds_dir = data_store_dir;
     if( !fc::exists( data_store_dir ) )
     {
        if( create ) 
        {
           fc::create_directories( data_store_dir );
        }
        else
        {
           FC_THROW_EXCEPTION( file_not_found_exception, 
           "Unable to find data store directory '${dir}'",
           ("dir", data_store_dir) );
        }
     }
     my->out_by_addr_tbl.load( data_store_dir / "out_by_addr.table", create );
  }

  /**
   *  Syncs all memory-mapped files.
   */
  void deterministic_data_store::save()
  {
     my->out_by_addr_tbl.save(); 
  }

  /**
   *  Calculates the combined hash of all dividend and transfer output tables
   */
  fc::sha224 deterministic_data_store::calculate_hash()const
  {
    fc::sha224::encoder enc;
    fc::raw::pack( enc, get_index() );
    return enc.result();
  }

  const dds_index&           deterministic_data_store::get_index()const
  {
    my->ddsindex.by_address_header = my->out_by_addr_tbl.get_header();
    return my->ddsindex;
  }

  /*
  dividend_table&            deterministic_data_store::get_dividend_table( const unit_type& u )
  {

  }
  */

  output_by_address_table&   deterministic_data_store::get_output_by_address_table()
  {
    return my->out_by_addr_tbl; 
  }




} } // bts::dds
