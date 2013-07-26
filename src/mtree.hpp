#pragma once


/**
 *  T must support raw packing so that fc::sha256 may be calculated.
 *
 *  A mtree seralizes as an vector<T>
 */
template<typename T>
class mtree
{
  public:
     /**
      *  Inserts a new item, v, into the set.
      */
     fc::sha256   insert( const T& v )
     {
         uint32_t index = data.size();
         if( free_heap.size() )
         {
         }

         if( index >= data.size() )
         {
            data.push_back( v );
         }
         else
         {
            data[index] = v;
            dirty.insert(index);
         }
     }

     /**
      *  Gets an item from the tree by key
      */
     const T&     get( const fc::sha256& key )const
     {
          auto itr = index.find(key);
          if( itr == index.end() )
          {
             FC_THROW_EXCEPTION( fc::key_not_found_exception, "Key Not Found" );
          }
          assert( itr->second < data.size() );
          return data[itr->second];
     }
     void         remove( const fc::sha256& key )
     {
          auto itr = index.find(key);
          if( itr == index.end() )
          {
             FC_THROW_EXCEPTION( fc::key_not_found_exception, "Key Not Found" );
          }
          dirty.insert(itr->second); 
          free_heap.push_back(itr->second);
     }

     
     /**
      *   Calculates the mroot based upon the dirty
      *   elements.
      */
     fc::sha256   calculate_mroot();

  private:
     std::vector<uint32_t>                     free_heap; // list of free ids in the data array.
     std::set<uint32_t>                        dirty; // set of index in the hash tree that is dirty
     std::unordered_map<fc::sha256,uint32_t>   index; // index in data
     std::vector<T>                            data;
     std::vector<fc::sha256>                   hash_tree;

};
