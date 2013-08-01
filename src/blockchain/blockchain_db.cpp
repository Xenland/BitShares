#include <bts/config.hpp>
#include <bts/blockchain/trx_validation_state.hpp>
#include <bts/blockchain/blockchain_db.hpp>
#include <bts/blockchain/asset.hpp>
#include <leveldb/db.h>
#include <bts/db/level_map.hpp>
#include <fc/io/enum_type.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/io/raw.hpp>

#include <fc/filesystem.hpp>
#include <fc/log/logger.hpp>

#include <algorithm>

namespace bts { namespace blockchain {
    namespace ldb = leveldb;
    namespace detail  
    { 

      class blockchain_db_impl
      {
         public:
            blockchain_db_impl()
            :head_block_num(-1),current_bitshare_supply(0){}

            //std::unique_ptr<ldb::DB> blk_id2num;  // maps blocks to unique IDs
            bts::db::level_map<fc::sha224,uint32_t>             blk_id2num;
            bts::db::level_map<uint160,trx_num>                 trx_id2num;
            bts::db::level_map<trx_num,meta_trx>                meta_trxs;
            bts::db::level_map<uint32_t,block>                  blocks;
            bts::db::level_map<uint32_t,std::vector<uint160> >  block_trxs; 

            uint32_t                                            head_block_num;
            uint64_t                                            current_bitshare_supply;
            // Dividend Table needs to be memory mapped
      };
    }

     blockchain_db::blockchain_db()
     :my( new detail::blockchain_db_impl() )
     {
     }

     blockchain_db::~blockchain_db()
     {
     }

     void blockchain_db::open( const fc::path& dir, bool create )
     {
       try {
         if( !fc::exists( dir ) )
         {
              if( !create )
              {
                 FC_THROW_EXCEPTION( file_not_found_exception, 
                     "Unable to open name database ${dir}", ("dir",dir) );
              }
              fc::create_directories( dir );
         }
         my->blk_id2num.open( dir / "blk_id2num", create );
         my->trx_id2num.open( dir / "trx_id2num", create );
         my->meta_trxs.open(  dir / "meta_trxs",  create );
         my->blocks.open(     dir / "blocks",     create );
         my->block_trxs.open( dir / "block_trxs", create );
         
         block blk;
         // read the last block from the DB
         my->blocks.last( my->head_block_num, blk );

         my->current_bitshare_supply  = blk.state.issuance.data[asset::bts].issued;
         my->current_bitshare_supply += calculate_mining_reward( my->head_block_num ) / 2;

       } FC_RETHROW_EXCEPTIONS( warn, "error loading blockchain database ${dir}", ("dir",dir)("create",create) );
     }

     void blockchain_db::close()
     {
        my->blk_id2num.close();
        my->trx_id2num.close();
        my->blocks.close();
        my->block_trxs.close();
        my->meta_trxs.close();
     }

    uint32_t blockchain_db::head_block_num()const
    {
       return my->head_block_num;
    }

    /**
     *  @pre trx must pass evaluate_signed_transaction() without exception
     *  @pre block_num must be a valid block 
     *
     *  @param block_num - the number of the block that contains this trx.
     *
     *  @return the index / trx number that was assigned to trx as part of storing it.
     */
    void  blockchain_db::store_trx( const signed_transaction& trx, const trx_num& trx_idx )
    {
       try {
         my->trx_id2num.store( trx.id(), trx_idx );
         
         meta_trx mt(trx);
         mt.meta_outputs.resize( trx.outputs.size() );
         my->meta_trxs.store( trx_idx, mt );

       } FC_RETHROW_EXCEPTIONS( warn, 
          "an error occured while trying to store the transaction", 
          ("trx",trx) );
    }

    trx_num    blockchain_db::fetch_trx_num( const uint160& trx_id )
    {
       return my->trx_id2num.fetch(trx_id);
    }
    meta_trx    blockchain_db::fetch_trx( const trx_num& trx_id )
    {
       return my->meta_trxs.fetch( trx_id );
    }

    uint32_t    blockchain_db::fetch_block_num( const fc::sha224& block_id )
    {
       return my->blk_id2num.fetch( block_id ); 
    }

    block       blockchain_db::fetch_block( uint32_t block_num )
    {
       return my->blocks.fetch(block_num);
    }

    full_block  blockchain_db::fetch_block_trxs( uint32_t block_num )
    {
       full_block fb = my->blocks.fetch(block_num);
       fb.trx_ids = my->block_trxs.fetch( block_num );
       return fb;
    }

    /**
     *  Calculate the dividends due to a given asset accumulated durrning blocks from_num to to_num
     */
    asset              blockchain_db::calculate_dividends( const asset& a, uint32_t from_num, uint32_t to_num )
    {
       return asset();
    }
    /**
     *  The most recent blocks do not pay dividends, except to the miner, becaues the dividends
     *  would be lost in a chain reorg.  
     *
     *  @return only the dividends, not the balance
     */
    asset      blockchain_db::calculate_dividend_fees( const asset& b, uint32_t from_num )
    {
       return asset();
    }

    /**
     *  Returns all dividends due to an output with balance b in block from_num not
     *  including dividends from the last 100 blocks.
     *
     *  @return only the dividends paid, not including the initial balance
     */
    asset      blockchain_db::calculate_output_dividends( const asset& b, uint32_t from_num )
    {
       return asset();
    }


    std::vector<meta_trx_input> blockchain_db::fetch_inputs( const std::vector<trx_input>& inputs )
    {
       try
       {
          std::vector<meta_trx_input> rtn;
          rtn.reserve( inputs.size() );
          for( uint32_t i = 0; i < inputs.size(); ++i )
          {
            try {
             trx_num tn   = fetch_trx_num( inputs[i].output_ref.trx_hash );
             meta_trx trx = fetch_trx( tn );
             
             if( inputs[i].output_ref.output_idx >= trx.meta_outputs.size() )
             {
                FC_THROW_EXCEPTION( exception, "Input ${i} references invalid output from transaction ${t}",
                                    ("i",inputs[i])("o", trx) );
             }
             if( inputs[i].output_ref.output_idx >= trx.outputs.size() )
             {
                FC_THROW_EXCEPTION( exception, "Input ${i} references invalid output from transaction ${t}",
                                    ("i",inputs[i])("o", trx) );
             }

             meta_trx_input metin;
             metin.source       = tn;
             metin.output_num   = inputs[i].output_ref.output_idx;
             metin.output       = trx.outputs[metin.output_num];
             metin.meta_output  = trx.meta_outputs[metin.output_num];
             rtn.push_back( metin );

            } FC_RETHROW_EXCEPTIONS( warn, "error fetching input [${i}] ${in}", ("i",i)("in", inputs[i]) );
          }
          return rtn;
       } FC_RETHROW_EXCEPTIONS( warn, "error fetching transaction inputs", ("inputs", inputs) );
    }


    /**
     *  Validates that trx could be included in a future block, that
     *  all inputs are unspent, that it is valid for the current time,
     *  and that all inputs have proper signatures and input data.
     *
     *  @return any trx fees that would be paid if this trx were included
     *          in the next block.
     *
     *  @throw exception if trx can not be applied to the current chain state.
     */
    trx_eval blockchain_db::evaluate_signed_transaction( const signed_transaction& trx )       
    {
       try {
           trx_validation_state vstate( trx, fetch_inputs( trx.inputs ), this ); 
           vstate.validate();

           trx_eval e;
           if( vstate.balance_sheet[asset::bts].out > vstate.balance_sheet[asset::bts].in )
           {
              e.coinbase =  vstate.balance_sheet[asset::bts].out - vstate.balance_sheet[asset::bts].in;
           }
           else
           {
              e.fees = vstate.balance_sheet[asset::bts].in - vstate.balance_sheet[asset::bts].out;
              e.fees += vstate.dividend_fees;
           }

           return e;
       } FC_RETHROW_EXCEPTIONS( warn, "error evaluating transaction ${t}", ("t", trx) );
    }


    void validate_initial_issuance( const fc::array<asset_issuance, asset::type::count>& isu )
    {
       for( uint32_t i = 0; i < asset::type::count; ++i )
       {
         FC_ASSERT( isu.at(i).backing == 0 );
         FC_ASSERT( isu.at(i).issued  == 0 );
       }
       FC_ASSERT( isu.data[0].backing == 0 );
       FC_ASSERT( isu.data[0].issued  == 0 );
    }

    void validate_mining_reward( const full_block& new_blk, const block& prev_blk )
    {
       uint64_t reward = new_blk.state.issuance.data[0].issued - prev_blk.state.issuance.data[0].issued;
       if( new_blk.block_num == 0 )
       {
         FC_ASSERT( reward == 0 );
       }
       else
       {
         /** the block state contains the initial conndition for the new block, or the
          *  post condition of the prior block.  We want to make sure that the new block
          *  records the proper increase in BTS from the mining reward of the prior block,
          *  and thus the need to subtract 1 from the new_blk.block_num
          */
         FC_ASSERT( reward == calculate_mining_reward( new_blk.block_num - 1 ) );
       }
    }

    
    /**
     *  Attempts to append block b to the block chain with the given trxs.
     */
    void blockchain_db::push_block( const trx_block& b )
    {
      try {
        FC_ASSERT( b.version == 0  );
        FC_ASSERT( b.trxs.size() > 0 );
        block prev_blk;
        
        // TODO: no need to lookup last block num... we can
        // pull it from cache...
        fc::sha224 last_blk_id;
        uint32_t   last_blk_num = 0;
        if( my->blk_id2num.last( last_blk_id, last_blk_num ) )
        {
          FC_ASSERT( b.block_num == last_blk_num + 1 );
          prev_blk = my->blocks.fetch( last_blk_num );
          FC_ASSERT( prev_blk.block_num == last_blk_num );
        }
        else
        {
          FC_ASSERT( b.block_num == 0 );
          FC_ASSERT( b.prev      == fc::sha224() );
          validate_initial_issuance( b.state.issuance );
        }
        
        validate_mining_reward( b, prev_blk );
        
        // TODO: factor this loop out into separate method, it is needed
        // in multiple places and would make this method more readable
        trx_eval total_eval;
        for( auto itr = b.trxs.begin(); itr != b.trxs.end(); ++itr )
        {
            total_eval += evaluate_signed_transaction( *itr );
        }
        ilog( "summary: ${totals}", ("totals",total_eval) );
        
        auto new_bts      = calculate_mining_reward(b.block_num);
        
        // TODO: calculate fees and add them into the coinbase / dividend calculation

        auto new_div      = new_bts / 2;
        auto new_coinbase = new_bts - new_div; /* capture any rounding errors from div 2 */

        // TODO: verify that the dividends came out in the b.state.dividend_percent

        if( total_eval.coinbase != asset( new_coinbase, asset::bts) )
        {
           FC_THROW_EXCEPTION( exception, "block has invalid coinbase amount, expected ${e}, but created ${c}",
                               ("e", new_coinbase)("c",total_eval.coinbase) );
        }

        my->current_bitshare_supply += new_bts;

        // TODO: actually push the block!
        // TODO: update my->head_block_num
        
      } FC_RETHROW_EXCEPTIONS( warn, "unable to push block", ("b", b) );
    }

    /**
     *  Removes the top block from the stack and marks all spent outputs as 
     *  unspent.
     */
    void blockchain_db::pop_block( full_block& b, std::vector<signed_transaction>& trxs )
    {
    }

    struct trx_stat
    {
       uint16_t trx_idx;
       trx_eval eval;
    };
    bool operator < ( const trx_stat& a, const trx_stat& b )
    {
      return a.eval.fees.amount < b.eval.fees.amount;
    }

    uint64_t blockchain_db::current_bitshare_supply()
    {
       return my->current_bitshare_supply; // cache this every time we push a block
    }

    /**
     *  First step to creating a new block is to take all canidate transactions and 
     *  sort them by fees and filter out transactions that are not valid.  Then
     *  filter out incompatible transactions (those that share the same inputs).
     *
     *  
     *
     */
    trx_block  blockchain_db::generate_next_block( const address& coinbase_addr, 
                                                   const std::vector<signed_transaction>& trxs )
    {
      try {
         FC_ASSERT( coinbase_addr != address() );
         std::vector<trx_stat>  stats;
         stats.reserve(trxs.size());
         
         for( uint32_t i = 0; i < trxs.size(); ++i )
         {
            try 
            {
                trx_stat s;
                s.eval = evaluate_signed_transaction( trxs[i] );

                if( s.eval.coinbase.amount != 0 )
                {
                  ilog( "ignoring transaction ${trx} because it creates coins", 
                        ("trx",trxs[i]) );
                }
                s.trx_idx = i;
                
                stats.push_back( s );
            } 
            catch ( const fc::exception& e )
            {
               ilog( "unable to use trx ${t}", ("t", trxs[i] ) );
            }
         }

         // order the trx by fees
         std::sort( stats.begin(), stats.end() ); 

         fc::datastream<size_t>  block_size;
         uint32_t conflicts = 0;

         asset total_fees;
         // make sure inputs are unique
         std::unordered_set<output_reference> consumed_outputs;
         for( size_t i = 0; stats.size(); ++i )
         {
            const signed_transaction& trx = trxs[stats[i].trx_idx]; 
            for( size_t in = 0; in < trx.inputs.size(); ++in )
            {
               if( !consumed_outputs.insert( trx.inputs[i].output_ref ).second )
               {
                    stats[i].trx_idx = uint16_t(-1); // mark it to be skipped, input conflict
                    ++conflicts;
                    break; //in = trx.inputs.size(); // exit inner loop
               }
            }
            if( stats[i].trx_idx != uint16_t(-1) )
            {
               fc::raw::pack( block_size, trx );
               if( block_size.tellp() > MAX_BLOCK_TRXS_SIZE )
               {
                  stats.resize(i); // this trx put us over the top, we can stop processing
                                   // the other trxs.
               }
            }
            total_fees += stats[i].eval.fees;
         }

         // at this point we have a list of trxs to include in the block that is sorted by
         // fee and has a set of unique inputs that have all been validated against the
         // current state of the blockchain_db, calculate the total fees paid, half of which
         // are paid as dividends, the rest to coinbase
         
         total_fees += asset(calculate_mining_reward( my->head_block_num + 1 ), asset::bts);

         asset miner_fees( (total_fees.amount / 2).high_bits(), asset::bts );
         asset dividends(0,asset::bts);
         
         // TODO: where do I put the dividends?
         uint64_t cur_bts_supply = current_bitshare_supply();
         if( cur_bts_supply != 0 ) // don't divide by 0
         {
            dividends.amount = (total_fees.amount - miner_fees.amount) / cur_bts_supply;
            FC_ASSERT( dividends.amount < fc::uint128(1,0) );
         }

         trx_block new_blk;
         new_blk.timestamp = fc::time_point::now();
         new_blk.state.dividend_percent = dividends.amount.low_bits();
         new_blk.trxs.reserve( 1 + stats.size() - conflicts ); 

         // create the coin base trx
         signed_transaction coinbase;
         coinbase.version = 0;
         coinbase.valid_after = 0;
         coinbase.valid_blocks = 0;

         coinbase.outputs.push_back( 
              trx_output( claim_by_signature_output( coinbase_addr ), 
                          miner_fees.amount.high_bits(), asset::bts) );

         new_blk.trxs.push_back( coinbase ); 

         // add all other transactions to the block
         for( size_t i = 0; i < stats.size(); ++i )
         {
           if( stats[i].trx_idx != uint16_t(-1) )
           {
             new_blk.trxs.push_back( trxs[ stats[i].trx_idx] );
           }
         }
         return new_blk;

      } FC_RETHROW_EXCEPTIONS( warn, "error generating new block" );
    }

}  } // bts::blockchain


