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



namespace bts { namespace blockchain {
    namespace ldb = leveldb;
    namespace detail  
    { 

      class blockchain_db_impl
      {
         public:
            blockchain_db_impl()
            :head_block_num(-1){}

            //std::unique_ptr<ldb::DB> blk_id2num;  // maps blocks to unique IDs
            bts::db::level_map<fc::sha224,uint32_t>             blk_id2num;
            bts::db::level_map<uint160,trx_num>                 trx_id2num;
            bts::db::level_map<trx_num,meta_trx>                meta_trxs;
            bts::db::level_map<uint32_t,block>                  blocks;
            bts::db::level_map<uint32_t,std::vector<uint160> >  block_trxs; 

            uint32_t                                            head_block_num;
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
         
         my->blocks.last( my->head_block_num );
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

       trx_eval total_eval;
       for( auto itr = b.trxs.begin(); itr != b.trxs.end(); ++itr )
       {
           total_eval += evaluate_signed_transaction( *itr );
       }
       ilog( "summary: ${totals}", ("totals",total_eval) );

       auto new_bts      = calculate_mining_reward(b.block_num);
       auto new_div      = new_bts / 2;
       auto new_coinbase = new_div;
       if( total_eval.coinbase != asset( new_coinbase, asset::bts) )
       {
          FC_THROW_EXCEPTION( exception, "block has invalid coinbase amount, expected ${e}, but created ${c}",
                              ("e", new_coinbase)("c",total_eval.coinbase) );
       }
      } FC_RETHROW_EXCEPTIONS( warn, "unable to push block", ("b", b) );
    }

    /**
     *  Removes the top block from the stack and marks all spent outputs as 
     *  unspent.
     */
    void blockchain_db::pop_block( full_block& b, std::vector<signed_transaction>& trxs )
    {
    }

}  } // bts::blockchain


