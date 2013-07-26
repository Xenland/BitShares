#pragma once
#include "address.hpp"
#include "blockchain.hpp"
#include "wallet.hpp"
#include "meta.hpp"
#include <vector>
#include <memory>
#include <unordered_set>

namespace detail { class account_impl; }

/**
 *  An account is a set of addresses and the
 *  transactions that refer to them. It also
 *  contains any outputs for which there are
 *  no transactions, as a 'starting balance'
 *  in the case that the node lacks the full
 *  transaction history of the chain.
 *
 *  An account may or may not have the private
 *  keys associated with it available and
 *  thus can be used to monitor a group of
 *  addresses and to calculate a balance.
 *
 *  Each account has its own wallet which may
 *  optionally contain the private keys used
 *  for the account.
 *
 *  An account can be managed entirely independantly
 *  from the blockchain.  When an account is first
 *  loaded, all transactions and outputs must be
 *  validated against the current block chain to
 *  make sure they are still valid / consistant. This
 *  also happens anytime a fork merges.
 */
class account
{
  public:
     account();
     ~account();

     enum open_mode
     {
          create, read_only, read_write
     };

     /** returns the wallet for this account so
      * that it may be used to sign transactions that
      * spend from this account.
      */
     wallet&                           get_wallet();
     wallet&                           load_wallet( const std::string& pass );
     void                              save_wallet( const std::string& pass );

     void                              load( const fc::path& account_dir, open_mode m = create );
     void                              save();

     void                              set_name( const std::string& name );
     std::string                       name()const;
                                       
     void                              add_address( const address& a );

     /** Gets the next new address that hasn't been used by this account,
      *  yet exists in the wallet.
      **/
     address                           get_new_address();

     /** Gets all addresses associated with this account regardless
      *  of whether or not we have the private key
      */
     const std::unordered_set<address>& get_addresses()const;

     /** helper method for searching get_addresses() for addr */
     bool contains( const address& addr );

  private:
     std::unique_ptr<detail::account_impl> my;
};


