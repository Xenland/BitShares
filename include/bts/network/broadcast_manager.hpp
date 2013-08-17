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
          std::unordered_set<Key>                 known_keys;

          std::unordered_map<Key,fc::time_point>  requested_values;
      };

      struct item_state
      {
        item_state()
        :pending_validation(false),valid(false){}

        fc::time_point        query_time;
        bool                  pending_validation;
        bool                  valid;
        fc::optional<Value>   value;
      };

      void  received_inventory_notice( const Key& k )
      {
         if( _inventory.find( k ) == _inventory.end() )
         {
           _query_queue.insert(k);
         }
      }

      uint32_t unknown_count()const
      {
        return _query_queue.size();
      }

      Key      query_next()
      {
         FC_ASSERT( _query_queue.size() != 0 );
         auto key = *_query_queue.begin();
         _inventory[key].query_time = fc::time_point::now();
         return key;
      }

      void  received( const Key& k, const Value& v )
      {
         item_state& state = _inventory[k];
         FC_ASSERT( !state.value, "duplicate value received", ("value",v) );
         state.value              = v;
         state.pending_validation = true;
         _validation_queue.push_back(k);
      }

      const fc::optional<Value>& get_value( const Key& key )
      {
          auto itr = _inventory.find(key);  
          if( itr != _inventory.end() )
            return _unknown_value;
          return itr->second.value;
      }

      uint32_t pending_validation_count()const
      {
        return _validation_queue.size();
      }

      const Value& pop_for_validation()
      {
         FC_ASSERT( _validation_queue.size() != 0 );
         auto key = _validation_queue.front();
         _validation_queue.pop_front();
         FC_ASSERT( _inventory[key].value );
         return *_inventory[key].value;
      }

      void validated( const Key& key, bool is_ok )
      {
         item_state& state = _inventory[key];
         FC_ASSERT( state.pending_validation == true );
         state.pending_validation            = false;
         state.valid                         = is_ok;     
         FC_ASSERT( !!state.value );
      }

      void  remove( const Key& key )
      {
          _inventory.erase(key);
          _query_queue.erase(key);
          auto new_end = std::remove( _validation_queue.begin(), _validation_queue.end(), key );
          _validation_queue.erase( new_end, _validation_queue.end() );
      }

      void remove_invalid()
      {
         for( auto itr = _inventory.begin(); itr != _inventory.end(); )
         {
           if( !itr->second.valid && !itr->second.pending_validation ) 
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

    private:
      fc::optional<Value>                   _unknown_value;
      std::unordered_map<Key,item_state>    _inventory;
      std::deque<Key>                       _validation_queue;
      std::unordered_set<Key>               _query_queue;
  };

} } 
