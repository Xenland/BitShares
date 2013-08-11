#include <bts/wallet_cache.hpp>
#include <bts/blockchain/blockchain_db.hpp>
#include <bts/db/level_map.hpp>
#include <fc/filesystem.hpp>
#include <fc/reflect/variant.hpp>

#include <unordered_map>
#include <map>

struct output_balance
{
   uint8_t  unit;
   uint64_t amount;
};

bool operator < ( const output_balance& a, const output_balance& b )
{
   if ( a.unit < b.unit ) return true; 
   if ( a.unit > b.unit ) return false; 
   if ( a.amount < b.amount ) return true; 
   return false; 
}

FC_REFLECT( output_balance, (unit)(amount) )

namespace bts {
  using namespace blockchain;

  namespace detail 
  {
     class wallet_cache_impl
     {
        public:
          blockchain_db_ptr                            _block_db;
          db::level_map<uint32_t, extended_public_key> _accounts;
          // these two DB contain all information necessary to 
          db::level_map<uint160, signed_transaction>   _trx_db;

          /** index to quickly see if a public key is in our set. */
          db::level_map<bts::address, address_index>   _addr_index;  

          /** maps account number to the last trx used by the account */
          db::level_map<uint32_t,uint32_t>             _account_last_trx_index;

          /** maps account.trx number to the last recv used */
          db::level_map<uint64_t,uint32_t>             _trx_last_address_index;

          /** store all outputs that reference a particular address derived from
           *  our HD Wallet, in this case the value is the index of the value[i].output_idx 
           *  is really the input idx because the data for an input ref is the same
           *  as for an output ref.
           */
          std::unordered_map<output_reference, std::vector<output_reference> > _spent_outputs;

          /** stores all unspent outputs that can be claimed by a signature sorted by
           * unit & value.  This is rebuilt / updated every time a new block is added
           */
          std::map<output_balance, output_reference> _unspent_claim_by_sig;

          // TODO:  index spendable.... 
          
          // TODO:  index margin (short) positions

          // TODO:  index option positions
          
          // TODO:  index password outputs... 

          // TODO:  index open orders
     };
  } // namespace detail

  wallet_cache::wallet_cache( const blockchain_db_ptr& block_db )
  :my( new bts::detail::wallet_cache_impl() )
  {
    my->_block_db = block_db;
  }

  wallet_cache::~wallet_cache()
  {
  }

  void wallet_cache::open( const fc::path& wallet_cache_dir )
  { try {
      if( !fc::exists( wallet_cache_dir ) )
      {
         fc::create_directories( wallet_cache_dir );
      }

      my->_accounts.open( wallet_cache_dir / "accounts", true );
      my->_trx_db.open( wallet_cache_dir / "trx_db", true );
      my->_addr_index.open( wallet_cache_dir / "addr_index", true );
      my->_account_last_trx_index.open( wallet_cache_dir / "acnt_last_trx_index", true );
      my->_trx_last_address_index.open( wallet_cache_dir / "trx_last_address_index", true );
      
      // TODO generate _spent_outputs cache... 
      // TODO generate unspent_claim_by_sig cache

  } FC_RETHROW_EXCEPTIONS( warn, "", ("wallet_cache_dir",wallet_cache_dir) ) }


  /**
   *  This method will scan the trx for inputs or outputs
   *  that belong to this wallet.
   */
  void  wallet_cache::cache( const blockchain::signed_transaction& trx )
  {
     // TODO:  scan trx for addresses that belong to this wallet.
     my->_trx_db.store( trx.id(), trx );
  }

  asset wallet_cache::get_balance( asset::type unit )const
  {
    return asset();
  }

  void wallet_cache::add_account( uint32_t account, const extended_public_key& pub )
  { try {

     my->_accounts.store( account, pub );
     index_account_trx( account, 10, 10 ); // TODO: don't hardcode these numbers

  } FC_RETHROW_EXCEPTIONS( warn, "error adding account ${a} = ${k}", ("a", account)("k",pub) ) }

  void wallet_cache::remove_account( uint32_t account )
  { try {

     my->_accounts.remove( account );

  } FC_RETHROW_EXCEPTIONS( warn, "error removing account ${a} = ${k}", ("a", account) ) }

  void wallet_cache::rescan_chain()
  {
  }

  void wallet_cache::index_account_trx( uint32_t account, uint32_t trx_limit, uint32_t adr_limit )
  {
  }

  void wallet_cache::index_account_trx_addresses( uint32_t account, uint32_t trx, uint32_t adr_limit )
  {
  }

} // namespace bts

