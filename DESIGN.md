Account:
  - contains a set of addresses and all transactions that reference them.
  - contains a wallet that may have some or all of the private keys for the account.

Wallet: 
  - contains a set of private keys and the matching public keys.  Manages Encryption 
  and security of the private keys.

BlockChain:
  - contains all known blocks + the current longest block.
  - validates all transactions / transfers.
  - applies validated blocks.
  - builds new blocks.

Market:
  - used by the BlockChain, organizes / manages all transactions
    associated with a buy/sell pair.

Miner:
  - given a block will attempt to solve the proof-of-work.

Node:
  - Provides communication with other nodes, receives transactions / blocks,
    and will broadcast transactions / blocks.


Database:
  - contains every transaction we have ever received along with the following meta-information:
         - which blocks it was included in.
  - contains every block, combined with meta information regarding whether it is in the chain, and any 'next' blocks
  - contains every output_id mapped to the trx id / block id that it was included in



Given a set of 'bid' outputs and a set of 'ask' outputs:
   Grab all bid outputs and place them on the old-bid stack.
   Grab all ask outputs and place them on the old-ask stack.

   Find highest bid and lowest ask able to meet the minimum.
      - make an exchange at the average of the bid/ask price
            - add the result of the exchange to the balance of each 'address'
            - place the 'change' into the change-bid and change-ask stack.

   Find the highest bid from the change/old stacks
   Find the lowest ask from the change/old stacks
      - add the result of the exchange to the balance of each 'address'
      - push the 'change' back onto the output stack.
      
   repeat until there are no asks able to satisfiy any bids.

   Generate a new transaction that takes all bids/asks as inputs and
   creates new outputs for the exchange results + unmatched change.


   The blockchain would then look like:

        bid 100 BS for 1 USD
        bid 101 BS for 1 USD
        bid 102 BS for 1 USD
        ask 90 BS for 1 USD
        ask 95 BS for 1 USD
        ask 96 BS for 1 USD (short)

    Match 102 vs 96 to get 99 strike price
    Match 101 vs 95 to get 98 strike price
    Match 100 vs 90 to get 95 strike price

       There would be 6 inputs and 6 outputs for this transaction and 0 change.

       3 people would receive spendable USD
       2 people would receive spendable BS
       1 person would receive collateralized BS redeemable in USD at exchange rate

       There would only be 1 output per address per unit and claim type.  

       This algorithm must be deterministic from the current state of the
       block chain.

   Because the bid/ask matching algorithm is determistic, that means that bids
   and asks carry the full fee and get placed into the chain according to their 
   fee.    Because people are watching these bids/asks they will potentially
   want to 're-broadcast' their bid/ask with a higher fee to 'replace' their
   eariler bids/asks.  The result is that 'day-traders' inter-block price
   volatility will cause bids/asks to be retracted at a 'higher fee' and thus
   drive fees/dividends for everyone when the bids/asks are finally entered
   into the blockchain. 
     
   Result: 'double spends' on the inputs to bids will be allowed to 
   flow through the network provided they include a higher fee. Higher
   fees benefit everyone through dividends. 
