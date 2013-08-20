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
      broadcast_manager()
      :_new_since_broadcast(false){}

      class channel_data 
      {
         public:
           void update_known( const std::vector<Key>& known )
           {
             for( auto itr = known.begin(); itr != known.end(); ++itr )
             {
               _known_keys[*itr] = fc::time_point::now();
             }
           }
           void clear_old( const fc::time_point& old )
           {
             for( auto itr = _known_keys.begin(); itr != known_keys.end(); )
             {
                if( *itr < old )
                {
                  itr= _known_keys.erase(itr);
                }
                else
                {
                  ++itr;
                }
             }
           }

           bool did_request( const Key& k )const
           {
              return _requested_values.find(k) != _requested_values.end();
           }
           void received_response( const Key& k )
           {
              FC_ASSERT( did_request( k ) );
              _requested_values.erase(k);
           }
           bool knows( const Key& k )const
           {
             return _known_keys.find(k) != _known_keys.end();
           }
           bool has_pending_request()const
           {
             return _requested_values.size() != 0;
           }
           void requested( const Key& k )
           {
              _requested_values[k] = fc::time_point::now();
           }
           const std::unordered_map<Key,fc::time_point>& known_keys()const
           {
             return _known_keys;
           }
         private:
          std::unordered_map<Key,fc::time_point>  _known_keys;
          std::unordered_map<Key,fc::time_point>  _requested_values;
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

      bool find_next_query( Key& key )const
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
        return high_count != 0;
      }

      void  item_queried( const Key& key )
      {
          _inventory[key].query_time = fc::time_point::now();
          _inventory[key].inv_count  = -10000; // flag so we don't query again
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
         FC_ASSERT( !state.value, "duplicate value received", ("value",value)("current",*state.value)("key",key) );
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
      std::vector<Key> get_inventory( const channel_data& filter )
      {
         std::vector<Key> unique_items; 
         unique_items.reserve( _inventory.size() );

         for( auto itr = _inventory.begin(); itr != _inventory.end(); ++itr )
         {
           if( itr->second.value && itr->second.valid )
           {
               if( filter.known_keys().find( itr->first ) == filter.known_keys().end() )
               {
                  unique_items.push_back( itr->first ); 
               }
           }
         }
         return unique_items;
      }
      std::vector<Value> get_inventory_values()const
      {
         std::vector<Value> unique_items; 
         unique_items.reserve( _inventory.size() );

         for( auto itr = _inventory.begin(); itr != _inventory.end(); ++itr )
         {
           if( itr->second.value && itr->second.valid )
           {
               unique_items.push_back( *itr->second.value ); 
           }
         }
         return unique_items;
      }

      bool has_new_since_broadcast()const
      {
        return _new_since_broadcast;
      }
      void set_new_since_broadcast( bool s )
      {
        _new_since_broadcast = s;
      }

      void clear_inventory()
      {
         _new_since_broadcast = false;
         _inventory.clear();
      }

      /**
       *  Called when a new block is pushed, old trxs are
       *  no longer valid in the current context, but may
       *  still be fetched if someone is downloading them
       */
      void invalidate_all()
      {
         for( auto itr = _inventory.begin(); itr != _inventory.end(); ++itr )
         {
           itr->second.valid = false;
         }
      }

      void clear_old_inventory()
      {
         // TODO: define a global inventory window time??  make it a parameter?
         fc::time_point old = fc::time_point::now() - fc::seconds( 60*10 );
         for( auto itr = _inventory.begin(); itr != _inventory.end(); )
         {
            if( itr->second.recv_time < old )
            {
               itr = _inventory.erase( itr );
            }
            else
            {
               ++itr;
            }
         }
      }

    private:
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


      bool                                  _new_since_broadcast;
      fc::optional<Value>                   _unknown_value;
      std::unordered_map<Key,item_state>    _inventory;
  };

} } 
