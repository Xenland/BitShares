#pragma once
#include <deque>
#include <unordered_map>
#include <fc/exception/exception.hpp>
#include <fc/reflect/variant.hpp>

namespace bts { namespace network {

  /**
   *  Abstracts the process of managing data that is broadcast
   *  via the traditional inventory, fetch, validate, relay
   *  process.
   */
  template<typename Key, typename Value>
  class broadcast_manager
  {
    public:
      struct channel_data 
      {
          void update_known( const std::vector<Key>& known )
          {
            for( auto itr = known.begin(); itr != known.end(); ++itr )
            {
               known_keys.insert(*itr);
            }
          }
          bool did_request( const Key& k )const
          {
             return requested_values.find(k) != requested_values.end();
          }
          void received_response( const Key& k )
          {
             FC_ASSERT( did_request( k ) );
             requested_values[k] = fc::time_point();
          }
          std::unordered_set<Key>                 known_keys;
          std::unordered_map<Key,fc::time_point>  requested_values;
      };

      struct item_state
      {
        item_state()
        :inv_count(0),valid(false){}

        int32_t               inv_count; ///< how many inventory msgs have I received
        fc::time_point        recv_time;
        fc::time_point        query_time;
        bool                  valid;
        fc::optional<Value>   value;
      };

      void  received_inventory_notice( const Key& k )
      {
         auto itr = _inventory.find(k);
         if( itr != _inventory.end() )
         {
           ++itr->second.inv_count;
         }
         else
         {
           _inventory[k].inv_count = 1;
         }
      }

      bool find_next_query( Key& key )
      {
        int32_t high_count = 0;
        for( auto itr = _inventory.begin(); itr != _inventory.end(); ++itr )
        {
          if( itr->second.inv_count > high_count )
          {
             high_count = itr->second.inv_count;
             key = itr->first;
          }
        }
        if( high_count != 0 )
        {
          _inventory[key].query_time = fc::time_point::now();
          _inventory[key].inv_count  = -10000; // flag so we don't query again
          return true;
        }
        return false;
      }

      const fc::optional<Value>& get_value( const Key& key )
      {
          auto itr = _inventory.find(key);  
          if( itr != _inventory.end() )
            return _unknown_value;
          return itr->second.value;
      }

      /**
       *  Called after a key/value has been validated with the result.  This
       *  will add the key to our inventory.
       */
      void validated( const Key& key, const Value& value, bool is_ok )
      {
         item_state& state = _inventory[key];
         FC_ASSERT( !state.value, "duplicate value received", ("value",value) );
         state.recv_time = fc::time_point::now();
         state.value     = value;
         state.valid     = is_ok;

         _new_since_broadcast = true;
      }

      void  remove( const Key& key )
      {
          _inventory.erase(key);
      }

      void remove_invalid()
      {
         fc::time_point expire_time = fc::time_point::now() - fc::seconds(60); 
         for( auto itr = _inventory.begin(); itr != _inventory.end(); )
         {
           if( !itr->second.valid && itr->second.recv_time < expire_time )
           {
              itr = _inventory.erase(itr);
           }
           else
           {
              ++itr;
           }
         }
      }
      
      /**
       *  @return a vector of validated keys that does not contain any items already 
       *          found in filter.
       */
      std::vector<Key> get_inventory( const std::unordered_set<Key>& filter )
      {
         std::vector<Key> unique_items; 
         unique_items.reserve( _inventory.size() );

         for( auto itr = _inventory.begin(); itr != _inventory.end(); ++itr )
         {
           if( itr->second.value && itr->second.valid )
           {
               if( filter.find( itr->first ) == filter.end() )
               {
                  unique_items.push_back( itr->first ); 
               }
           }
         }
         return unique_items;
      }

      bool has_new_since_broadcast()
      {
        return _new_since_broadcast;
      }
      void set_new_since_broadcast( bool s )
      {
        _new_since_broadcast = s;
      }

      broadcast_manager()
      :_new_since_broadcast(false){}

    private:
      bool                                  _new_since_broadcast;
      fc::optional<Value>                   _unknown_value;
      std::unordered_map<Key,item_state>    _inventory;
  };

} } 
