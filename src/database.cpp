#include "database.hpp"
#include <unordered_map>
#include <unordered_set>

namespace detail
{
  class database_impl
  {
    public:
      std::unordered_map<pow_hash,meta_block_ptr>                  _blocks;

      /** maps a block_id to a set of block_id's that come after it */
      std::unordered_map<pow_hash,std::unordered_set<pow_hash> >   _prev_index;
      std::unordered_map<fc::sha224,meta_signed_transaction_ptr>   _trxs;
      std::unordered_map<fc::sha224,meta_output_ptr>               _outputs;
      std::unordered_map<address, std::unordered_set<fc::sha224> > _outputs_by_claim;
  };
}


database::database()
:my( new detail::database_impl() )
{

}

database::~database(){}

void database::load( const fc::path& db_dir )
{
}

void database::close()
{
}

void database::store( const block& b, const pow_hash& h )
{
   auto itr = my->_blocks.find( h );
   if( itr == my->_blocks.end() )
   {
      meta_block_ptr mb = std::make_shared<meta_block>();
      mb->head = b.header;
      for( auto itr = b.trxs.begin(); itr != b.trxs.end(); ++itr )
      {
        store( *itr, h );
        mb->trxs.push_back( itr->calculate_id() );
      }
      my->_blocks[h] = mb;
   }
   my->_prev_index[b.header.prev_block].insert( h );
}

meta_signed_transaction_ptr database::store( const signed_transaction& trx, const pow_hash& block_id )
{
   auto tid = trx.calculate_id();
   auto itr = my->_trxs.find( tid );
   if( itr == my->_trxs.end() )
   {
      meta_signed_transaction_ptr mst = std::make_shared<meta_signed_transaction>();
      mst->trx = trx;
      mst->blk = block_id;
      my->_trxs[tid] = mst;

      // add all of the outputs
      for( uint32_t i = 0; i < trx.outputs.size(); ++i )
      {
          auto oid = output_cache( tid, i, trx.outputs[i] ).output_id;
          my->_outputs[oid] = std::make_shared<meta_output>( trx.outputs[i], tid );

      }
      return mst;
   }
   return itr->second;
}


meta_block_ptr                   database::fetch_block( const pow_hash& block_id )
{
  return my->_blocks[block_id];
}
meta_signed_transaction_ptr      database::fetch_transaction( const fc::sha224& trx_id )
{                                
  return my->_trxs[trx_id];      
}                                
meta_output_ptr              database::fetch_output( const fc::sha224& output_id )
{
  return my->_outputs[output_id];
}

std::vector<meta_output_ptr> database::fetch_outputs_by_address( const address& adr )
{
   std::vector<meta_output_ptr> out;
   auto itr = my->_outputs_by_claim.find(adr);
   
   if( itr == my->_outputs_by_claim.end() )
   {
     return out;
   }

   for( auto i = itr->second.begin(); i != itr->second.end(); ++i )
   {
      auto trxo = fetch_output( *i );
      if( trxo ) out.push_back(trxo);
   }
   return out;
}




