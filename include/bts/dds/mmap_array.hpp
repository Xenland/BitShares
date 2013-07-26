#pragma once
#include <fc/exception/exception.hpp>
#include <fc/interprocess/file_mapping.hpp>
#include <fc/filesystem.hpp>
#include <fc/io/fstream.hpp>
#include <fc/log/logger.hpp>
#include <memory>

namespace bts 
{ 
  /**
   *  Maps an array of POD types from a file to memory
   */
  template<typename T>
  class mmap_array
  {
      struct file_data
      {
         file_data():_size(0){};
         size_t                             _size;
         T                                  _data[1];
      };

      public:
         mmap_array()
         :_data(nullptr){}

         ~mmap_array()
         {
            try
            {
               close();
            } 
            catch ( ... ) 
            {
                wlog( "unexpected exception" );
            }
         }
         void close()
         {
            if( _mapped_region )
            {
              _mapped_region->flush();
              _mapped_region.reset(nullptr);
              _mapped_file.reset(nullptr);
            }
         }
         
         void open( const fc::path& f )
         {
            try {
              if( !fc::exists( f ) )
              { 
                 fc::ofstream o( f );
                 file_data init;
                 init._size = 0;
                 o.write( (char*)&init, sizeof(init) );
              }
              auto fs = fc::file_size(f);
              _file = f;
              _mapped_file.reset( new fc::file_mapping( f.generic_string().c_str(), fc::read_write ) );
              _mapped_region.reset( new fc::mapped_region( *_mapped_file, fc::read_write) );
              _data = (file_data*)_mapped_region->get_address();
              FC_ASSERT( _data != nullptr );
              
              if( _data->_size * sizeof(T) > (fs-sizeof(size_t)) )
              {
                 FC_THROW_EXCEPTION( exception, "file ${f} is corrupted", ("f",f));
              }
              // TODO: any other sanity checks?? 
            } FC_RETHROW_EXCEPTIONS( warn, "unable to open mmap_array ${f}", ("f", f) );
         }
         
         void resize( size_t s )
         {
            try 
            {
               FC_ASSERT( !!_mapped_region ); 
               _mapped_region->flush();
               _mapped_region.reset(nullptr);
               _mapped_file.reset(nullptr);
               
               if( s > 0 )
               {
                   fc::resize_file( _file, s*sizeof(T) + sizeof(size_t) );
               }
               else
               {
                   fc::resize_file( _file, sizeof(file_data) );
               }
               
               this->open( _file );
               
               _data->_size = s;
            } FC_RETHROW_EXCEPTIONS( warn, "resizing mmap_array to ${s}", ("s",s) );
         }
         
         T& at( uint64_t index )
         {
            FC_ASSERT( index < _data->_size );
            FC_ASSERT( _data != nullptr );
            return _data->_data[index]; 
         }
         const T& at( uint64_t index )const
         {
            FC_ASSERT( index < _data->_size );
            FC_ASSERT( _data != nullptr );
            return _data->_data[index]; 
         }

         size_t size()const 
         { 
            FC_ASSERT( nullptr != _data );
            return _data->_size; 
         }

      private:
         fc::path                           _file;
         file_data*                         _data;
         std::unique_ptr<fc::file_mapping>  _mapped_file;
         std::unique_ptr<fc::mapped_region> _mapped_region;
  };

} // namespace bts  
