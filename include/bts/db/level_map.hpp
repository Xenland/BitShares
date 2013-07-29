#pragma once
#include <leveldb/db.h>
#include <leveldb/comparator.h>
#include <fc/filesystem.hpp>
#include <fc/reflect/reflect.hpp>
#include <fc/io/raw.hpp>
#include <fc/exception/exception.hpp>

namespace bts { namespace db {

  namespace ldb = leveldb;

  /**
   *  @brief implements a high-level API on top of Level DB that stores items using fc::raw / reflection
   *
   *
   *  @note Key must be a POD type
   */
  template<typename Key, typename Value>
  class level_map
  {
     public:
        void open( const fc::path& dir, bool create )
        {
           ldb::Options opts;
           opts.create_if_missing = create;
           opts.comparator = & _comparer;
           
           ldb::DB* ndb = nullptr;
           auto ntrxstat = ldb::DB::Open( opts, dir.generic_string().c_str(), &ndb );
           if( !ntrxstat.ok() )
           {
               FC_THROW_EXCEPTION( exception, "Unable to open database ${db}\n\t${msg}", 
                    ("db",dir)
                    ("msg",ntrxstat.ToString()) 
                    );
           }
           _db.reset(ndb);
        }

        void close()
        {
          _db.reset();
        }

        Value fetch( const Key& k )
        {
          try {
             ldb::Slice ks( (char*)&k, sizeof(k) );
             std::string value;
             auto status = _db->Get( ldb::ReadOptions(), ks, &value );
             if( !status.ok() )
             {
                 FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
             }
             fc::datastream<const char*> ds(value.c_str(), value.size());
             Value tmp;
             fc::raw::unpack(ds, tmp);
             return tmp;
          } FC_RETHROW_EXCEPTIONS( warn, "error fetching key ${key}", ("key",k) );
        }

        bool last( Key& k, Value& v )
        {
          try {
           std::unique_ptr<ldb::Iterator> it( _db->NewIterator( ldb::ReadOptions() ) );
           FC_ASSERT( it != nullptr );
           it->SeekToLast();
           if( !it->Valid() )
           {
             return false;
           }
           fc::datastream<const char*> ds( it->value().data(), it->value().size() );
           fc::raw::unpack( ds, v );

           fc::datastream<const char*> ds2( it->key().data(), it->key().size() );
           fc::raw::unpack( ds2, k );
           return true;
          } FC_RETHROW_EXCEPTIONS( warn, "error reading last item from database" );
        }

        void store( const Key& k, const Value& v )
        {
          try
          {
             ldb::Slice ks( (char*)&k, sizeof(k) );
             auto vec = fc::raw::pack(v);
             ldb::Slice vs( vec.data(), vec.size() );
             
             auto status = _db->Put( ldb::WriteOptions(), ks, vs );
             if( !status.ok() )
             {
                 FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
             }
          } FC_RETHROW_EXCEPTIONS( warn, "error storing ${key} = ${value}", ("key",k)("value",v) );
        }

        void remove( const Key& k )
        {
          try
          {
            ldb::Slice ks( (char*)&k, sizeof(k) );
            auto status = _db->Delete( ldb::WriteOptions(), ks );
            if( !status.ok() )
            {
                FC_THROW_EXCEPTION( exception, "database error: ${msg}", ("msg", status.ToString() ) );
            }
          } FC_RETHROW_EXCEPTIONS( warn, "error removing ${key}", ("key",k) );
        }
        

     private:
        class key_compare : public leveldb::Comparator
        {
          public:
            int Compare( const leveldb::Slice& a, const leveldb::Slice& b )const
            {
               FC_ASSERT( (a.size() == sizeof(Key)) && (b.size() == sizeof( Key )) );
               Key* ak = (Key*)a.data();        
               Key* bk = (Key*)b.data();        
               if( *ak  < *bk ) return -1;
               if( *ak == *bk ) return 0;
               return 1;
            }

            const char* Name()const { return "key_compare"; }
            void FindShortestSeparator( std::string*, const leveldb::Slice& )const{}
            void FindShortSuccessor( std::string* )const{};
        };

        key_compare                  _comparer;
        std::unique_ptr<leveldb::DB> _db;
        
  };


} } // bts::db
