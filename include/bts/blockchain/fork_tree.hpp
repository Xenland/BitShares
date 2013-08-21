#pragma once
#include <fc/exception/exception.hpp>
#include <fc/optional.hpp>

#include <vector>
#include <map>

namespace bts { namespace blockchain {

  /**
   *  This data structure tracks forks so that the
   *  longest, highest difficulty chain can be fetched
   *  and we can track which headers have been evaluated
   *  and found wanting.  
   */
  template<typename Key>
  class fork_tree
  {
      private: // private first for template ordering..
         struct node_data
         {
             typedef std::shared_ptr<node_data> ptr;
             node_data(){}
             node_data( const Key& id, const Key& pre )
             :node_id(id),prev_id(pre),votes(1),difficulty(0){}

             /** calculates the maximum depth of children links from this 
              * node.
              */
             uint32_t calc_depth()const
             {
                uint32_t max_depth = 0;
                for( auto c = children.begin(); c != children.end(); ++c )
                {
                   // TODO: use non-recursive algo to save stack!
                   uint32_t d = (*c)->calc_depth();
                   if( d > max_depth ) max_depth = d;
                }
                return 1 + max_depth;
             }

             void set_valid( bool valid_state )
             {
               if( valid && *valid == valid_state ) return; // no change
               if( !valid_state )
               {
                 valid = valid_state;
                 for( auto c = children.begin(); c != children.end(); ++c )
                 {
                   (*c)->valid.reset(); // TODO: use non recursive algorithm to save stack overflow 
                 }
                 return;
               }
               valid = valid_state;
             }

             Key                            node_id;
             Key                            prev_id;
             uint32_t                       votes; ///< track how many peers report this fork as 'master'
             uint64_t                       difficulty; ///< cache difficulty, TODO: use this, rather than height to select best
             fc::optional<bool>             valid;     
             std::vector< typename node_data::ptr >  children;
         };
         std::map<uint32_t, std::vector<typename node_data::ptr> >  _nodes;
      public:
         void check_node( uint32_t height, const Key& node_id )
         { try {
            FC_ASSERT( _nodes.find(height) != _nodes.end() );
            std::vector<typename node_data::ptr>& nodes_at_height = _nodes[height];
            for( uint32_t i = 0; i < nodes_at_height.size(); ++i )
            {
               if( nodes_at_height[i]->node_id == node_id )
               {
                 return;
               }
            }
            FC_ASSERT( !"nothing found" );
         }  FC_RETHROW_EXCEPTIONS( warn, "unable to find node ${id} at height ${height}", ("id",node_id)("height",height) ) }
         
         void add_node( uint32_t height, const Key& node_id, const Key& prev_id )
         {
            std::vector<typename node_data::ptr>& nodes_at_height = _nodes[height];
            for( uint32_t i = 0; i < nodes_at_height.size(); ++i )
            {
               if( nodes_at_height[i]->node_id == node_id )
               {
                 nodes_at_height[i]->votes++;
                 return;
               }
            }
            auto new_node = std::make_shared<node_data>( node_id, prev_id );
            nodes_at_height.push_back( new_node );
            
            auto prev_itr = _nodes.find(height-1);
            if( prev_itr != _nodes.end() )
            {
                std::vector<typename node_data::ptr>& nodes_at_prev_height = prev_itr->second;
                for( uint32_t i = 0; i < nodes_at_height.size(); ++i )
                {
                   if( nodes_at_prev_height[i]->node_id == prev_id )
                   {
                     nodes_at_prev_height[i]->children.push_back( new_node );
                     return;
                   }
                }
            }
         }
          
         /** recursively sets the invalid flag on all children if invalid */
         void set_valid_state( uint32_t height, const Key& node_id, bool valid_state )
         {
            std::vector<typename node_data::ptr>& nodes_at_height = _nodes[height];
            for( uint32_t i = 0; i < nodes_at_height.size(); ++i )
            {
               if( nodes_at_height[i]->node_id == node_id )
               {
                 nodes_at_height[i]->set_valid( valid_state );
                 return;
               }
            }
            FC_THROW_EXCEPTION( key_not_found_exception, "unable to find node id ${id} at block number ${height}", ("id",node_id)("height",height)("valid_state",valid_state) );
         }

         fc::optional<Key> get_best_fork_for_height( uint32_t height )
         {
            auto nodes_at_height_itr = _nodes.find(height);
            if( nodes_at_height_itr != _nodes.end() ) 
            {
              return fc::optional<Key>();
            }
            std::vector<typename node_data::ptr>& nodes_at_height = nodes_at_height_itr->second;
            if( nodes_at_height.size() == 0 )
            {
              return fc::optional<Key>();
            }

            uint32_t best_idx    = 0;
            uint32_t best_height = 0;
            uint32_t best_votes  = 0;
            for( uint32_t i = 0; i < nodes_at_height.size(); ++i )
            {
               uint32_t node_depth = nodes_at_height[i]->calc_depth();
               bool better = false;
               if( node_depth > best_height )
               {
                  better = true;
               }
               else if( node_depth == best_height )
               {
                  if( nodes_at_height[i]->votes > best_votes )
                  {
                    better = true;
                  }
               }

               if( better )
               {
                  best_idx = 0;
                  best_height = node_depth;
                  best_votes  = nodes_at_height[i]->votes;
               }
            }
            return nodes_at_height[best_idx]->node_id;
         }


  };


} } // bts::blockchain
