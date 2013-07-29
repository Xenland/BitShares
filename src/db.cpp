#include <leveldb/db.h>
#include <fc/filesystem.hpp>
#include <fc/exception/exception.hpp>

namespace bts 
{
   namespace ldb = leveldb;
   std::unique_ptr<ldb::DB> init_db( const fc::path& dbfile, bool create )
   {
      ldb::Options opts;
      opts.create_if_missing = create;

      ldb::DB* ndb = nullptr;
      auto ntrxstat = ldb::DB::Open( opts, dbfile.generic_string().c_str(), &ndb );
      if( !ntrxstat.ok() )
      {
          FC_THROW_EXCEPTION( exception, "Unable to open database ${db}\n\t${msg}", 
               ("db",dbfile)
               ("msg",ntrxstat.ToString()) 
               );
      }

      return std::unique_ptr<ldb::DB>(ndb);
   }

} // bts
