#include "api.hpp"
#include "config.hpp"
#include "meta.hpp"
#include "proof_of_work.hpp"
#include "chain_state.hpp"
#include <fc/io/json.hpp>
#include <list>
#include <unordered_set>
#include <assert.h>
#include <sstream>
#include <iostream>

namespace detail
{
    class block_chain_impl
    {
       public:
         std::vector<meta_block_header> _chain;
         block                          _unconfirmed_head;
         unsigned char*                 _pow_buffer;
         chain_state                    _state;


         std::unordered_map<pow_hash,block> _block_db;

         void validate_exchange( const std::map<unit,uint64_t>& in_value, 
                                 const std::map<unit,uint64_t>& out_value,
                                 const std::vector<output_cache>& inputs,
                                 const signed_transaction& trx );

         uint64_t apply_transactions( const block& b );
         uint64_t apply_transaction( const signed_transaction& trx );
         void check_block_dividends( const block& b, uint64_t total_fees );
         void claim_output( const trx_output& out, const std::vector<char>& in, const signed_transaction& trx );
         uint64_t calculate_dividends( const output_cache& out );
         uint64_t calculate_dividend_fee( const output_cache& out );
    };
}

fc::sha224 transaction::calculate_id()const
{
  fc::sha224::encoder enc;
  fc::raw::pack( enc, *this );
  return enc.result();
}

output_cache::output_cache( const fc::sha224& trx_id, uint32_t idx, const trx_output& out )
:output_state(out)
{
  fc::sha224::encoder enc;
  fc::raw::pack( enc, trx_id );
  fc::raw::pack( enc, idx );
  fc::raw::pack( enc, out );
  output_id = enc.result();
}


block_chain::block_chain()
:my( new detail::block_chain_impl() )
{
}


block_chain::~block_chain()
{
}

void block_chain::load( const fc::path& data_dir )
{
   if( !fc::exists( data_dir ) ) 
      fc::create_directories(data_dir);
   
   // if there is no chain... generate the gensis block
   generate_gensis_block();
}


const block& block_chain::get_unconfirmed_head()
{
    return my->_unconfirmed_head;
}

void  block_chain::generate_gensis_block()
{
   // generate gensis block and add it

   block b;
   b.header.version.value         = 0;
   b.header.miner_version.value   = 0;
   b.header.timestamp             = fc::time_point::from_iso_string("20130608T002349");
   b.header.height                = my->_chain.size();
   b.header.prev_block            = pow_hash(); // null
   b.header.nonce                 = 0;
   b.header.dividends             = 0;

   b.header.dividends += get_reward_for_height(0);

   // generate coin-base transaction

   signed_transaction coinbase_trx;
   coinbase_trx.outputs.push_back( trx_output( b.header.dividends, unit() ) );
   coinbase_trx.outputs.back().set_claim_function( claim_with_coinbase_address( address("abcd") ) ); // TODO: define a real address here!
   b.trxs.emplace_back(std::move(coinbase_trx));

   output_cache oc( b.trxs.back().calculate_id(), 0, b.trxs.back().outputs.back() );
   my->_state.addOutput( oc );
   my->_state.commit();
   b.header.block_state = my->_state.root;
    
   // store transaction in TRX DB
   my->_chain.push_back( meta_block_header() );
   my->_chain.back().header = b.header;
   my->_chain.back().id = proof_of_work( my->_chain.back().header, my->_pow_buffer );

   my->_block_db[my->_chain.back().id] = b;
}


block  block_chain::generate_next_block( const address& a )
{
   block b;
   b.header.version.value         = 0;
   b.header.miner_version.value   = 0;
   b.header.timestamp             = fc::time_point::now();
   b.header.height                = my->_chain.size();
   b.header.prev_block            = my->_chain.back().id;
   b.header.nonce                 = 0;
   b.header.dividends             = 0;

   // apply all non-market transactions

   // perform market calculations
   
   // calculate dividends and updated shares.

   b.header.dividends += get_reward_for_height(b.header.height) / 2;
   // generate coin-base transaction
   signed_transaction coinbase_trx;
   coinbase_trx.outputs.push_back( trx_output( b.header.dividends, unit() ) );
   // TODO: validate all 'coinbase transactions' use claim_with_coinbase flag
   ilog( "claim_with_coinbase_address ${address}", ("address",a) );
   coinbase_trx.outputs.back().set_claim_function( claim_with_coinbase_address( a ) );
   ilog( "   check: ${address}", ("address", coinbase_trx.outputs.back().get_claim_address() ) );
   b.trxs.emplace_back(std::move(coinbase_trx));
   ilog( "   block check: ${address}", ("address", b.trxs.back().outputs.back().get_claim_address() ) );
   ilog( "..............................\n");
   

   // TODO: temp commit changes to find the state...
   output_cache oc( b.trxs.back().calculate_id(), 0, b.trxs.back().outputs.back() );
   my->_state.addOutput( oc );
   ilog( "..............................\n");

   // save the state..
   auto undo_state = my->_state.commit();
   wlog( "   commit block check: ${address}", ("address", b.trxs.back().outputs.back().get_claim_address() ) );
   b.header.block_state = my->_state.root;
   // go back to prior state... 
   my->_state.undo(undo_state);
   wlog( "   undo block check: ${address}", ("address", b.trxs.back().outputs.back().get_claim_address() ) );

   
   return b;
}

void  block_chain::add_block( const block& b )
{
  ilog( "...... " );
   // no matter what we need to know the hash of the block.
   auto pow     = proof_of_work( b.header, my->_pow_buffer ); 
   my->_block_db[pow] = b;

   // is this a fork of an earlier point in the chain or another chain all togeher?
   if( b.header.height <  my->_chain.size() )
   {
       wlog( "not next in the chain..." );
       return;
   }

   if( b.header.height > my->_chain.size() )
   {
       wlog( "we appear to have missed something\n" );
       return;
   }

   if( my->_chain.back().id != b.header.prev_block )
   {
      // TODO: un-hinged... perhaps part of a forked-chain
      //       
   }
   else
   {
       try
       {
          int64_t total_fees = 0;
          //my->check_block_coinbase( b );            // verify only one coinbase transaction
          //my->check_block_inputs_unique( b );       // verify all inputs all unique
          total_fees += my->apply_transactions( b );  // make sure all transfers are valid
          my->check_block_dividends( b, total_fees ); // verify fees + reward == dividends / 2
          
          // attempt to apply all transactions... 
          // if successful commit those transactions to the state
          auto undo = my->_state.commit();
          // should I store this someplace... I think I will 
       } 
       catch ( ... )
       {
          my->_state.rollback(); // toss all unapplied changes
       }

       // validate that the result of the commit == b.block_state
      
       my->_chain.back().next_blocks.push_back(pow);
       my->_chain.push_back( meta_block_header() );
       my->_chain.back().id = pow;
       my->_chain.back().header = b.header;
   }
}

uint64_t block_chain::current_difficulty()
{
  // calculate the average time for the past 120 blocks
  // calculate the delta percent from 10 minutes
  // adjust the difficulty by that percent.
  // difficulty of 1 = average difficulty found by single CPU in 10 minutes
  return 1;
}
int64_t  block_chain::get_reward_for_height( int64_t h )
{
   // TODO: unit-test this method
   int64_t reward = INIT_BLOCK_REWARD * SHARE;
   do
   {
      ilog( "height: ${h}  ${reward}", ("h", h)("reward",reward) );
         int num  = h > REWARD_ADJUSTMENT_INTERVAL ? REWARD_ADJUSTMENT_INTERVAL : h;
         reward -= num * (reward/2/REWARD_ADJUSTMENT_INTERVAL);
      ilog( "height: ${h}  ${reward}", ("h", h)("reward",reward) );
      h -= REWARD_ADJUSTMENT_INTERVAL;
   } while( h > 0 );
  ilog( "height: ${h}  ${reward}", ("h", h)("reward",reward) );

   return reward;
}



/*
void detail::block_chain_impl::check_block_inputs_unique( const block& b )
{
    std::unordered_set<fc::sha224> input_set;
    for( auto itr = b.trx.begin(); itr != b.trx.end(); ++itr )
    {
       for( auto in = itr->inputs.begin(); in != itr->inputs.end(); ++in )
       {
           if( !input_set.insert( in->source_output ) )
           {
              FC_THROW_EXCEPTION( exception, "duplicate inputs" ); 
           }
       }
    }
}
*/

/**
 *  All transactions that are not part of a bid/ask must have matching
 *  units and must have more inputs than outputs (a non-0 fee)
 */
uint64_t detail::block_chain_impl::apply_transactions( const block& b )
{
    uint64_t total_fees = 0;
    int      coinbase_count = 0;
    for( auto itr = b.trxs.begin(); itr != b.trxs.end(); ++itr )
    {
       if( itr->inputs.size() != 0 ) // not a coinbase
       {
          // TODO: assert trx has not expired by this block
          total_fees += apply_transaction( *itr );
       }
       else // this is a coinbase, make sure there is only 1
       {
          if( coinbase_count > 0 )
          { 
              // TODO: indicate that this is a malicious block and flag
              // who ever sent this to us!
              FC_THROW_EXCEPTION( exception, "multiple coinbase!" );
          }
       }
    }
    return total_fees;
}


uint64_t detail::block_chain_impl::calculate_dividends( const output_cache& out )
{
  return 0;
}
uint64_t detail::block_chain_impl::calculate_dividend_fee( const output_cache& out )
{
  return 0;
}


/**
 *   A transfer requires that all inputs and outputs have matching value
 */
bool is_transfer( std::map<unit,uint64_t>& in, std::map<unit,uint64_t>& out, uint64_t total_fee )
{
    for( auto itr = in.begin(); itr != in.end(); ++itr )
    {
       auto out_itr = out.find(itr->first);
       if( out_itr == out.end() )
       {
          return false;
       }
       if( itr->second != out_itr->second )
       {
          // check to see if the fee makes them equal... 
          if( itr->first == unit() && out_itr->second == (total_fee+itr->second) )
          {
             continue;
          }
          return false;
       }
    }
    return true;
}



/**
 *  @return total fees
 */
uint64_t detail::block_chain_impl::apply_transaction( const signed_transaction& trx )
{
    std::vector<output_cache>  inputs;
    inputs.reserve( trx.inputs.size() );

    uint64_t dividend_due  = 0; // dividends earned over 24 hours ago
    uint64_t dividend_fee  = 0; // dividends earned in the past 24 hours

    std::map<unit,uint64_t> in_value; // track all inputs by value

    // calculate the total value of all inputs, including dividends + fees
    // store the 'outputs' so we can validate exchange rates (if any) later
    for( auto itr = trx.inputs.begin(); itr != trx.inputs.end(); ++itr )
    {
       output_cache& out = _state.getOutput( itr->source_output );

       // validate that we can actually claim this output, it must be in the chain state,
       // and be part of a valid transaction.
       claim_output( inputs.back().output_state, itr->claim_input, trx );

       dividend_due           +=  calculate_dividends( out );
       dividend_fee           +=  calculate_dividend_fee( out );

       if( in_value.find( out.output_state.amount_unit ) != in_value.end() )
       {
           in_value[ out.output_state.amount_unit ] += out.output_state.amount;
       }
       else
       {
           in_value[ out.output_state.amount_unit ] =  out.output_state.amount;
       }
       inputs.push_back( out );
    }
    in_value[unit()] = dividend_due;

    // calculate the output value in each unit
    std::map<unit,uint64_t> out_value; // track all outputs by value.
    in_value[unit()] = 0;

    for( auto itr = trx.outputs.begin(); itr != trx.outputs.end(); ++itr )
    {
       if( out_value.find( itr->amount_unit ) != out_value.end() )
           out_value[ itr->amount_unit ] += itr->amount;
       else
           out_value[ itr->amount_unit ] = itr->amount;
    }

    // calculate total fees earned by this transaction
    int64_t total_fees =  dividend_fee + (in_value[unit()] - out_value[unit()]);
    if( total_fees <= 0 ) 
    {
      FC_THROW_EXCEPTION( exception, "No Fees Paid" ); 
    }

    // check that we have a valid transfer or exchange, note that 'bids' are
    // just transfers that have extra claim criteria
    if( !is_transfer( in_value, out_value, total_fees ) )
    {
        // checks to make sure this exchange is happening at the proper price
        // *note* to claim the inputs the transaction already has proper outputs
        // so all that this must do is check that the exchange rate / margin
        // requirements are met... 
        validate_exchange( in_value, out_value, inputs, trx );
    }

    // this should have been validated by is_transfer or validate exchange
    assert( in_value[unit()] > out_value[unit()] );

    // remove all inputs from the current state
    for( auto itr = inputs.begin(); itr != inputs.end(); ++itr )
    {
      _state.removeOutput( itr->output_id );
    }

    fc::sha224 trx_id = trx.calculate_id();
    // add all outputs to the current state
    for( uint32_t i = 0; i < trx.outputs.size(); ++i )
    {
       _state.addOutput( output_cache( trx_id, i, trx.outputs[i] ) ); 
    }

    return total_fees;
}

void detail::block_chain_impl::check_block_dividends( const block& b, uint64_t total_fees )
{
  // TODO
}

/**
 *  Applies input to the output claim function in the context of trx.  Throws an exception if
 *  the input / trx do not satisfy the claim criteria
 *
 *  @throw an exception if the output could not be claimed!
 */
void detail::block_chain_impl::claim_output( const trx_output& out, const std::vector<char>& in, const signed_transaction& trx )
{
  // TODO     
}

void detail::block_chain_impl::validate_exchange( const std::map<unit,uint64_t>& in_value, 
                                                  const std::map<unit,uint64_t>& out_value,
                                                  const std::vector<output_cache>& inputs,
                                                  const signed_transaction& trx )
{
  // TODO
}

/**
 *  Note this method may through if public_key recovery fails for one of the provided signtures.
 */
std::vector<address> signed_transaction::get_signed_addresses()const
{
  auto trx_id       = calculate_id();
  //ilog( "trx_id: ${d}", ("d", trx_id) );
  fc::sha256 digest = fc::sha256::hash( (char*)&trx_id, sizeof(trx_id) );
  //ilog( "digest: ${d}", ("d", digest) );

  std::vector<address> addrs;
  for( auto itr = trx_sigs.begin(); itr != trx_sigs.end(); ++itr )
  {
    addrs.push_back( address( fc::ecc::public_key( *itr, digest ) ) );
  }
  return addrs;
}
void    signed_transaction::sign_with_key( const fc::ecc::private_key& pk )
{
  auto trx_id       = calculate_id();
  //ilog( "trx_id: ${d}", ("d", trx_id) );
  fc::sha256 digest = fc::sha256::hash( (char*)&trx_id, sizeof(trx_id) );
  //ilog( "digest: ${d}", ("d", digest) );

  trx_sigs.push_back( pk.sign_compact( digest ) );
}

std::string   block_chain::pretty_print_output( const fc::sha224& out )
{
  return pretty_print_output( my->_state.getOutput(out) );
}
std::string   block_chain::pretty_print_output( const output_cache& out )
{
  /*
  std::stringstream ss;
    if( out.amount_unit == unit() )
        ss<<out.amount<< " bs ";
    else
        ss<<out.amount<< "  unit: "<< out.amount_unit.id <<" margin: "<<out.amount_unit.min_margin<<"%  ";
  
  return ss.str();
  */
  return pretty_print_output( out.output_state );
}


std::string   block_chain::pretty_print_output( const trx_output& out )
{
  std::stringstream ss;
    if( out.amount_unit == unit() )
        ss<<out.amount<< " bs ";
    else
        ss<<out.amount<< "  unit: "<< out.amount_unit.id <<" margin: "<<out.amount_unit.min_margin<<"%  ";
    ss<<"claim with ";
    switch( out.claim_function.value )
    {
        case claim_with_address::type:
        {
            ss<<"address "<< std::string(fc::raw::unpack<claim_with_address>( out.claim_data ).addr);
        }
        break;
        case claim_with_coinbase_address::type:
        {
            ss<<"coinbase address "<< 
                std::string(fc::raw::unpack<claim_with_coinbase_address>( out.claim_data ).addr);
        }
        break;
        case claim_with_exchange::type:
        {
            ss<< "exchange ";

        }
        break;
    }

  return ss.str();
}

/**
 *  @return a human-readable, well-formated view of the transaction.
 */
std::string   block_chain::pretty_print_transaction( const signed_transaction& trx )
{
  std::stringstream ss;
  ss<<" Transaction: "<< fc::string(trx.calculate_id()).c_str() <<"\n"; 
  ss<<"   Inputs: \n";

  ss<<"   Outputs: \n";
    int i = 0;
    for( auto itr = trx.outputs.begin(); itr != trx.outputs.end(); ++itr )
    {
       ss<<"    "<<pretty_print_output( *itr )<<"  id: "<< fc::string( output_cache( trx.calculate_id(), i, *itr).output_id ).substr(0,8).c_str()<<"\n";
       ++i;
    }

  ss<<"   Fees: \n";
  ss<<"   Signed By:  ";
  auto sadr = trx.get_signed_addresses();
  for( auto itr = sadr.begin(); itr != sadr.end(); ++itr )
  {
    ss<<" "<<std::string(*itr)<<",";
    ++itr;
  }
  return ss.str();
}

block block_chain::get_block( const pow_hash& block_id )
{
  auto itr = my->_block_db.find(block_id);
  if( itr != my->_block_db.end() ) return itr->second;
  FC_THROW_EXCEPTION( exception, "unable to find block ${block}", ("block",block_id) );
}

std::string block_chain::pretty_print_block( const pow_hash& block_id )
{
   std::stringstream ss;
   auto blk = get_block( block_id );
   ss << blk.header.height <<"]   "<<  fc::string( block_id ).substr(0,8).c_str() 
      << "  prev: "<< fc::string( blk.header.prev_block ).substr(0,8).c_str() 
      << " @  "<<fc::string(blk.header.timestamp).c_str()<<" "
      << " dividends  "<<blk.header.dividends<<" bs\n";
   //ss << std::string( fc::json::to_string( blk.header ) ) << "\n";
   for( auto itr = blk.trxs.begin(); itr != blk.trxs.end(); ++itr )
   {
      ss<<pretty_print_transaction( *itr )<<"\n";
   }
   return ss.str();
}

void block_chain::pretty_print_chain()
{
   for( auto itr = my->_chain.begin(); itr != my->_chain.end(); ++itr )
   {
      std::cout<< "----------------------------------------------------------------------\n";
      std::cout<< pretty_print_block( itr->id ).c_str();
   }
   std::cout.flush();
}

fc::optional<address> trx_output::get_claim_address()const
{
   ilog( "claim_function.value: ${v}", ("v",claim_function.value) );
   if( claim_function.value == claim_with_coinbase_address::type ||
       claim_function.value == claim_with_address::type )
   {
      auto a = fc::raw::unpack<claim_with_address>( claim_data ).addr;
      ilog( "claim address: ${a}", ("a", a));
      return a;
   }
   return fc::optional<address>();
}



bool operator==( const unit& a, const unit& b ){ return a.id == b.id && a.min_margin == b.min_margin; }
bool operator!=( const unit& a, const unit& b ){ return !(a==b); }
bool operator<( const unit& a, const unit& b ) { return a.id < b.id  || ((a.id == b.id) && (a.min_margin < b.min_margin)); }

std::vector<output_cache> block_chain::get_outputs_for_address( const address& a )const
{
  ilog( "${a}", ("a",a) );
  return my->_state.getCurrentOutputsForAddress( a );
}
