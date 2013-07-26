#pragma once
#include <fc/signal.hpp>
#include <fc/filesystem.hpp>
#include "blockchain.hpp"
#include <vector>
#include <memory>
#include <map>

namespace detail 
{
    class block_chain_impl;
}


class meta_output_cache;

/**
 *  @class block_chain
 *  @brief Encapsulates access to the blockchain databases
 *
 */
class block_chain
{
   public:
      block_chain();
      ~block_chain();

      /**
       *  Emitted anytime the block chain changes due to a new
       *  block or transaction.  Changes include the head 'unconfirmed' block.
       */
      fc::signal<void()>                    changed;

      /**
       *  Emitted anytime the a new output is added to the chain state.
       *  Accounts can 'observe' these changes to  update their balances.
       */
      fc::signal<void(const output_cache&)> output_added;

      /**
       *  Emitted anytime an output is 'spent' or otherwise removed from
       *  the chain state.  Accounts can 'observe' these changes to 
       *  update their balances.
       */
      fc::signal<void(const output_cache&)> output_removed;
      std::vector<output_cache> get_outputs_for_address( const address& a )const;

      void                    load( const fc::path& data_dir );

      /**
       *  Adds a new transaction to the chain 'free pool'
       */
      void                    add_transaction( const transaction& trx );
      transaction             get_transaction( const fc::sha256& trx_id );

      /**
       *  Returns any transactions referenced by a block in the database for
       *  which we do not have the trx ID.
       */
      std::vector<fc::sha256> get_missing_transactions( const fc::sha256& trx );

      /**
       *  Adds the block to the database, doesn't mean it goes in the head.
       */
      void                    add_block( const block& b );

      /**
       *  Returns the most recent block with all 'unconfirmed' transactions.
       *
       *  @param new_chain will start a new block-chain if there is no chain
       */
      const block&            get_unconfirmed_head();

      block                   get_block( const pow_hash& h );
      block                   generate_next_block( const address& a );
      void                    generate_gensis_block();

      /**
       *  @return a human-readable, well-formated view of the transaction.
       */
      std::string             pretty_print_transaction( const signed_transaction& trx );
      std::string             pretty_print_output( const trx_output& out );
      std::string             pretty_print_output( const output_cache& out );
      std::string             pretty_print_output( const fc::sha224& out );
      std::string             pretty_print_block( const pow_hash& block_id );
      void                    pretty_print_chain();

      /**
       *  Returns all balances for a particular address based upon confirmation status.
       */
      std::map<unit,int64_t>  get_balances( const address& a, int confirmations = -1 );
      share_state             get_share_info( uint64_t unit_id );
      uint64_t                current_difficulty();

      /**
       *  @return the current exchange state for 
       */
      exchange_state          get_exchange_info( uint64_t sell_unit, uint64_t buy_unit = 0);

      static int64_t          get_reward_for_height( int64_t h );

      /**
       *  Returns all outputs spendable by a particular address, this includes
       *  orders that could be canceled.
       */
      std::vector<output_cache> get_outputs( const address& a );

   private:
      std::unique_ptr<detail::block_chain_impl> my; 
};

