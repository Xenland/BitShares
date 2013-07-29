#pragma once
#include <memory>

namespace fc { class path; }
namespace leveldb { class DB; }
namespace bts 
{
   /**
    *   Helper method for initializing the ldb database with boilerplate code.
    */
   std::unique_ptr<leveldb::DB> init_db( const fc::path& dbfile, bool create );
} // namespace bts
