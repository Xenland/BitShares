#include <bts/wallet.hpp>
#include <bts/proof_of_work.hpp>
#include <fc/thread/thread.hpp>
#include <fc/io/raw.hpp>


namespace bts
{
   wallet::wallet()
   {}
   wallet::~wallet()
   {}

   /**
    *  The purpose of this method is to increase the 
    *  computational complexity of a brute force attack
    *  against a given seed.  This method should take
    *  about 1 second to run.
    */
   fc::sha256 stretch_seed( const fc::sha256& seed )
   {
      fc::thread t("stretch_seed");
      return t.async( [=]() {
          fc::sha256 last = seed;
          for( uint32_t i = 0; i < 10; ++i )
          {
              auto p = proof_of_work( last );  
              last = fc::sha256::hash( (char*)&p, sizeof(p) );
          }
          return last; 
      } ).wait();
   }

   fc::sha256 calc_sequence( const fc::ecc::public_key& mpub, uint32_t seq )
   {
      fc::sha256::encoder enc;
      fc::raw::pack( enc, mpub );
      fc::raw::pack( enc, seq );
      return enc.result();
   }

   void                 wallet::set_seed( const fc::sha256& seed, bool stretch )
   {
     _stretched_seed = stretch ? stretch_seed(seed) : seed;

     _master_priv = fc::ecc::private_key::generate_from_seed(_stretched_seed); 
     _master_pub  = _master_priv->get_public_key();
   }
                        
   void                 wallet::set_master_public_key( const fc::ecc::public_key& k )
   {
      _master_priv = nullptr;
      _master_pub = k;
   }

   fc::ecc::private_key    wallet::get_master_private_key()const
   {
      FC_ASSERT( _stretched_seed != fc::sha256() );
      return *_master_priv; 
   }

   fc::ecc::public_key  wallet::get_master_public_key()const
   {
      FC_ASSERT( !!_master_pub );
      return *_master_pub;
   }

   fc::ecc::public_key  wallet::get_public_key( uint32_t index )
   {
      FC_ASSERT( !!_master_pub );
      return _master_pub->mult( calc_sequence( *_master_pub, index) );
   }

   fc::ecc::private_key wallet::get_private_key( uint32_t index )
   {
      FC_ASSERT( _stretched_seed != fc::sha256() );
      FC_ASSERT( !!_master_pub );
      return fc::ecc::private_key::generate_from_seed( _stretched_seed, calc_sequence(*_master_pub, index) ); 
   }
}
