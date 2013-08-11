#pragma once
#include <bts/hd_wallet.hpp>
#include <bts/blockchain/blockchain_db.hpp>

namespace fc { class path; }

namespace bts  {

  namespace detail { class wallet_cache_impl; }


  struct address_index
  {
     address_index( uint32_t a = 0, uint32_t t = 0, uint32_t n = 0)
     :account_num(a),trx_num(t),address_num(n){}

     uint32_t account_num;
     uint32_t trx_num;
     uint32_t address_num;
    
     inline friend bool operator < ( const address_index& a, const address_index& b )
     {
         if( a.account_num < b.account_num ) return true;
         if( a.account_num > b.account_num ) return false;
         if( a.trx_num < b.trx_num         ) return true;
         if( a.trx_num > b.trx_num         ) return false;
         if( a.address_num < b.address_num ) return true;
         return false;
     }
  };




  /**
   *  The purpose of this class is the cache all of the transactions that
   *  are referenced by a HD wallet so that they can be used to build new
   *  transactions to 'spend', 'cover', 'bid', 'cancel', etc... 
   *
   *  Every time a new trx added to the blockchain (or pending) it should
   *  be passed to the add_to_cache method, if the trx is removed for
   *  a reorg then remove_from_cache should be called.
   *
   *  The wallet cache operates entirely with public keys and is not
   *  able to generate new accounts (which use private derivation)
   *
   *  Note: wallet_cache assumes a single master Hierarchical Deterministic
   *  wallet and all account, trx, and address indexes are relative to this
   *  single wallet.  
   */
  class wallet_cache
  {
     public:
       wallet_cache( const blockchain::blockchain_db_ptr& chain );
       ~wallet_cache();

       /**
        *  The password is used to decrypt the private key stored on disk.
        */
       void open( const fc::path& wallet_cache_dir );

       void add_account( uint32_t account, const extended_public_key& pub );
       void remove_account( uint32_t account );

       /**
        * Scan the blockchain for new transactions, usually after adding a new
        * account, loading a new wallet.  Generally speaking, the wallet will
        * automatically rescan recent blocks everytime a new address is used
        * and this is only required when searching for lost funds.
        */
       void rescan_chain();

       /**
        *  Add all trx keys from the last known trx num, and all address up to
        *  adr_limit to the address index to quickly identify outputs that are
        *  relevant to this wallet. 
        */
       void index_account_trx( uint32_t account, uint32_t trx_limit, uint32_t adr_limit = 20 );

       /**
        *  Only index this new account.
        */
       void index_account_trx_addresses( uint32_t account, uint32_t trx, uint32_t adr_limit );

       /**
        *  This method will scan the trx for inputs/outputs
        *  that belong to this wallet and update the indexes 
        *  accordingly.
        */
       void  cache( const blockchain::signed_transaction& trx );

       /**
        *  Calculates the balance available to this wallet type.
        */
       blockchain::asset get_balance( blockchain::asset::type unit )const;

       // TODO: add method for creating a new transaction that would
       // spend some of the available balance.

     private:
       std::unique_ptr<detail::wallet_cache_impl> my;
  };

} // namespace bts

FC_REFLECT( bts::address_index, (account_num)(trx_num)(address_num) )

