#pragma once
#include <memory>

namespace detail { class chain_state_impl; }

/**
 *  Maintains the set of 'unspent outputs' and which
 *  block they were found in. This class must determinstically
 *  add and remove outputs from the unspent set in such 
 *  a way that all nodes can reproduce this state by
 *  appying all transaction in the block chain.
 *
 *  When a transaction is applied, its inputs are always
 *  removed before the outputs are added.
 *
 *  To use this class the chain_state_transaction helper
 *  is used to provide 'rollback' and 'commit' support while
 *  attempting to apply changes to the block chain.
 *
 *  The 'chain state' can be entirely determined from the
 *  set of unspent outputs combined with all block-headers
 *  dating back to the oldest unspent output.
 *
 *  The chain state can only be 'modified' by adding or
 *  removing outputs. Each header includes the delta change
 *  in the balance of all shares/shorts.  
 *
 *  The chain_state only stores the hashes to the actual
 *  outputs along with the block index which included
 *  the output.  Actual outputs are managed by the database.
 *
 *  Internally the chain-state is managed as an array of
 *  output-hashes combined with a 'free-list' that 
 *  identifies empty slots in the array.  This array
 *  is hashed via a merkel tree which allows changes to
 *  be made to the output set without requiring rehashing
 *  the entire chain_state.
 *
 *  *note* merkel tree implementation is TBD
 */
class chain_state
{
   public:
     chain_state();
     ~chain_state();

     void save( const fc::path& loc );
     void load( const fc::path& loc );

     /** @return the state to include in the blockchain */
     fc::sha224 get_state()const;

     bool contains( const fc::sha224& out )const;
     int32_t get_block_num_for_output( const fc::sha224& out );

   private:
     /**
      * @return the index where the output was stored.
      */
     void add_output( const fc::sha224& out, uint32_t block_num );

     /** 
      *  @return the index where the output was stored, now null
      */
     void remove_output( const fc::sha224& out );

     friend class chain_state_transaction;
     std::unique_ptr<detail::chain_state_impl> my;
};


/**
 *  This transaction is like a database transaction, it captures
 *  modifications to chain_state in a manner than can be 'rolledback',
 *  'committed' and 'undone' as needed.  It can be thought of
 *  as a 'patch' to the chain_state that can be kept around to help
 *  manage applying and 'failing' of transactions to the blockchain.
 *
 *  When there is a chain-split this will be used to rapidly 'rewind'
 *  back to the old state so that the new chain may be applied. It
 *  can also be used to rapidly move forward in the chainstate.  
 */
class chain_state_transaction
{
    public:
     chain_state_transaction( chain_state& s );
     ~chain_state_transaction();

     // saves this transaction so it can be stored in the block db
     std::vector<char> pack()const;

     // loads this transaction, so that it can be loaded from the block db
     void              unpack( const char* vec, size_t len );

     void reset(); // drops all changes and restors state to s.get_state()

     fc::sha224 initial_condition()const; // after calling reset
     fc::sha224 final_condition()const; // after calling commit

     /** @return true if either this transaction or the underlying chain_state 
      *           contains out *and* it has not been removed by this state
      *           transaction.
      */
     bool contains( const fc::sha224& out )const;

     /** adds an output at the specified block_height */
     void add_output( const fc::sha224& out, uint32_t block_num );

     /** 
      *  @return the index where the output was stored, now null
      */
     void  remove_output( const fc::sha224& out );

     /**
      * Applies all changes from this transaction to chain state and
      * updates 'final_condition' to the new chain_state::get_state()
      */
     void commit();

     /**
      * given a chain_state in final_condition(), will restore it
      * to initial_condition()
      */
     void undo();
   private:
     std::unique_ptr<detail::chain_state_transaction_impl> my;
};
