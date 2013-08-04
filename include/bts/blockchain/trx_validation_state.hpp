#pragma once
#include <bts/blockchain/block.hpp>
#include <bts/blockchain/transaction.hpp>
#include <bts/blockchain/blockchain_db.hpp>

namespace bts { namespace blockchain {
  
    /**
     *  While validating a transaction certain things need to be tracked to make
     *  sure the transaction is valid.  Thus struct tracks this information.  
     *
     *  This state assumes the meta_trx_input is accurate.
     *
     *  TODO: add current bid/ask spread for all pairs in the block chain as this
     *        information is required for margin call inputs.
     *
     *  TODO: add a reference to the current dividend table so that inputs can
     *        be calculated using their full balance w/ dividends.
     */
    class trx_validation_state
    {
        public:
           /**
            * @param t - the transaction that is being validated
            *
            * @param head_idx - the head index to evaluate this
            * transaction against.  This should be the prior block
            * before the one t will be included in.
            */
           trx_validation_state( const signed_transaction& t, 
                                blockchain_db* d, 
                                bool enforce_unspent_in = true,
                                uint32_t  head_idx = -1
                                );

           trx_validation_state(){}
           
           /** tracks the sum of all inputs and outputs for a particular
            * asset type in the balance_sheet 
            */
           struct asset_balance
           {
              asset in;
              asset out;
              bool is_balanced()const { return (in - out).amount == 0; }
           };

           /** this is cost because validation shouldn't modify the trx and
            * is captured by value rather than reference just in case we need 
            * thread safety of some kind... for performance see if we can
            * capture it by reference later.
            */
           const signed_transaction            trx; // TODO make reference?
           std::vector<meta_trx_input>         inputs;
                                             
           std::vector<asset_balance>          balance_sheet; // validate 0 sum 
           std::vector<asset_issuance>         issue_sheet; // update backing info


           /** spending of some inputs depends upon the existance of outputs to the
            * transaction that meet certain conditions such as exchange rates. The
            * same output cannot satisify 2 different input requirements. As we
            * process inputs we track which outputs have been used and make sure
            * there are no duplicates.
            */
           std::unordered_set<uint8_t>         used_outputs;
           std::unordered_set<address>         signed_addresses;

           /**
            *  contains all addresses for which a signature is required,
            *  this is validated last with the exception of multi-sig or
            *  escrow inputs which have optional signatures.
            */
           std::unordered_set<address>         required_sigs;

           /** dividends earned in the past 100 blocks that are counted toward
             * transaction fees.
             */
           asset                               dividend_fees;
           asset                               dividends;


           blockchain_db*                      db;

           bool                                enforce_unspent;
           uint32_t                            ref_head;
           /** @throw an exception on error */
           void validate();
        private:
           static const uint16_t output_not_found = uint16_t(-1);
           uint16_t find_unused_bid_output( const claim_by_bid_output& b );
           uint16_t find_unused_sig_output( const address& a, const asset& bal );

           void validate_input( const meta_trx_input& );
           void validate_signature( const meta_trx_input& );
           void validate_bid( const meta_trx_input& );
           void validate_long( const meta_trx_input& );
           void validate_cover( const meta_trx_input& );
           void validate_opt( const meta_trx_input& );
           void validate_multi_sig( const meta_trx_input& );
           void validate_escrow( const meta_trx_input& );
           void validate_password( const meta_trx_input& );

           void validate_output( const trx_output& );
           void validate_signature( const trx_output& );
           void validate_bid( const trx_output& );
           void validate_long( const trx_output& );
           void validate_cover( const trx_output& );
           void validate_opt( const trx_output& );
           void validate_multi_sig( const trx_output& );
           void validate_escrow( const trx_output& );
           void validate_password( const trx_output& );
    };

} } // bts::blockchain
FC_REFLECT( bts::blockchain::trx_validation_state::asset_balance, (in)(out) )
FC_REFLECT( bts::blockchain::trx_validation_state, 
    (trx)
    (ref_head)
    (dividends)
    (issue_sheet)
    (balance_sheet)
    (used_outputs)
    (required_sigs)
    (signed_addresses)
    (dividend_fees)
)
