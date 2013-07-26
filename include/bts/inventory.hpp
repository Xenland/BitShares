#pragma once
#include <map>
#include <unordered_map>
#include <fc/time.hpp>
#include <bts/mini_pow.hpp>

namespace bts 
{
  
  template<typename T>
  class inventory
  {
    public:
      std::vector<mini_pow> get_inventory( fc::time_point s = fc::time_point(),
                                           fc::time_point e = fc::time_point::now() )const
      {
          std::vector<mini_pow> in;
          auto st = time_index.lower_bound(s);
          auto ed = time_index.upper_bound(e);

          while( st != ed )
          {
            in.push_back( st->second ); 
            ++st;
          }
          return in;
      }

      void                  clear_inventory( fc::time_point s = fc::time_point(),
                                           fc::time_point e = fc::time_point::now() )
      {
          auto st = time_index.lower_bound(s);
          auto ed = time_index.upper_bound(e);
          auto c = st;
          while( c != ed )
          {
            items.erase( c->second );
            ++c;
          }
          time_index.erase( st, ed );

      }
      void                  store( const mini_pow& p, const T& msg )              
      {
         items[p] = msg;
         time_index[fc::time_point::now()] = p;
      }

      const T&              get( const mini_pow& p )const
      {
         auto itr = items.find(p);
         if( itr == items.end() )
         {
            FC_THROW_EXCEPTION( key_not_found_exception, "unable to find inventory item ${key}", ("key",p) );
         }
         return itr->second;
      }
      bool contains( const mini_pow& p )const
      {
         return items.end() != items.find(p);
      }

      std::unordered_map<mini_pow, T>    items;
      std::map<fc::time_point,mini_pow> time_index;
  };
}
