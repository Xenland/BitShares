#include "chain_state.hpp"

struct output_state
{
   output_state():block_num(0){}

   fc::sha224 output_id;
   uint32_t   block_num;
};
FC_REFLECT( output_state, (output_id)(block_num) )

namespace detail 
{

   class chain_state_impl
   {
      public:
        std::vector<uint32_t>       _freestack;
        std::vector<output_state>   _outputs;

        std::unordered_map<fc::sha224,uint32_t> _index;
        fc::sha224                              _state;
   };

   class chain_state_transaction_impl
   {
      public:
        chain_state_transaction_impl( chain_state& c )
        :_cstate(c){}

        chain_state&                _cstate;

        struct action
        {
           bool         add; // true for add, false for remove
           output_state state;
        };

        std::vector<action>                      _actions;
        std::unordered_set<fc::sha224>           _added;
        std::unordered_set<fc::sha224>           _removed;
        bool                                     _committed;
   };
}



void chain_state::save( const fc::path& loc ){}
void chain_state::load( const fc::path& loc ){}

/** @return the state to include in the blockchain */
fc::sha224 chain_state::get_state()const
{
  return my->_state;
}

bool    chain_state::contains( const fc::sha224& out )const
{
  return my->_index.find(out) != my->_index.end();
}

int32_t chain_state::get_block_num_for_output( const fc::sha224& out )
{
  auto itr = my->_index.find(out);
  if( itr != my->_index.end() )
  {
    return _outputs[itr->second].block_num;
  }
  FC_THROW_EXCEPTION( key_not_found_exception, "unable to find output ${out}", ("out", fc::string(out)) );
}


void chain_state::add_output( const fc::sha224& out, uint32_t block_num )
{
    if( _freestack.size() == 0 )
    {
      _outputs.push_back( output_state(out,block_num) );
      my->_index[out] = _outputs.size() - 1;
      return;
    }

    auto idx = my->_freestack.back();
    my->_freestack.pop_back();
    my->_outputs[idx] = output_state(out, block_num);
    my->_index[out] = idx;
}

void chain_state::remove_output( const fc::sha224& out )
{
  auto itr = my->_index.find(out);
  if( itr != my->_index.end() )
  {
     my->_outputs[itr->second] = output_state();
     if( itr->second != my->_outputs.size() -1 )
     {
        my->_freestack.push_back(itr->second);
     }
     else 
     {
        my->_outputs.pop_back();
     }
     my->_index.erase(itr);
  }
}


chain_state_transaction::chain_state_transaction( chain_state& s )
:my( new detail::chain_state_transaction_impl(s) )
{
  my->_committed = false;
}

chain_state_transaction::~chain_state_transaction()
{
}

void chain_state_transaction::reset() 
{
  _actions.clear();

}

fc::sha224 chain_state_transaction::initial_condition()const
{
  return my->_init_state;
}
fc::sha224 chain_state_transaction::final_condition()const
{
  return my->_final_state;
}

/** @return true if either this transaction or the underlying chain_state 
 *           contains out *and* it has not been removed by this state
 *           transaction.
 */
bool chain_state_transaction::contains( const fc::sha224& out )const
{
}

/** adds an output at the specified block_height */
void chain_state_transaction::add_output( const fc::sha224& out, uint32_t block_num )
{
}

/** 
 *  @return the index where the output was stored, now null
 */
void  chain_state_transaction::remove_output( const fc::sha224& out )
{
}

/**
 * Applies all changes from this transaction to chain state and
 * updates 'final_condition' to the new chain_state::get_state()
 */
void chain_state_transaction::commit()
{
}

/**
 * given a chain_state in final_condition(), will restore it
 * to initial_condition()
 */
void chain_state_transaction::undo()
{
}
