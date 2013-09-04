#pragma once

/** 
 *  This is the file that contains all of the 'magic' numbers used throughout the
 *  code and provides them with a useful name.  This is the file where you can experiment
 *  with many arbitrary parameters such as timeouts, sizes, ports, etc.
 *
 *  Naming Scheme:
 *   COMPONENT_PROPERTY_UNIT
 *
 *  Valid Components:
 *    NETWORK
 *    PEER
 *    BITCHAT
 *    BITNAME     - bitname p
 *    BITSHARE    - properties of the bitshare blockchain
 *    RPC
 *    GUI
 *    BTS         - catch all
 *
 *  Group properties by component in the order listed above.
 *
 *  TODO: update configs below to follow scheme described above
 */

#define NETWORK_DEFAULT_PORT             (9876)
#define BITNAME_BLOCK_INTERVAL_SEC       (5*1)  // 5 minutes
#define BITNAME_TIMEKEEPER_WINDOW        (64)    // blocks used for estimating time
#define BITNAME_BLOCK_FETCH_TIMEOUT_SEC  (60)
#define RPC_DEFAULT_PORT              (NETWORK_DEFAULT_PORT+1)

#define SHARE                         (1000ll)                    // used to position the decimal place
#define MAX_BITSHARE_SUPPLY           (2000000000000ll * SHARE)   // 1 Trillion with 3 decimal places
#define YEARS_TO_MINE                 (12ll)                      // how many years until the full bitshare supply is mined
#define BLOCK_INTERVAL                (5ll)                       // minutes between blocks
#define BLOCKS_PER_HOUR               (60/BLOCK_INTERVAL)           
#define BLOCKS_PER_DAY                (BLOCKS_PER_HOUR*24ll)
#define BLOCKS_PER_YEAR               (BLOCKS_PER_DAY*365ll)
#define BLOCKS_WITH_REWARD            (1250000ll)
#define INITIAL_REWARD                ((2ll * MAX_BITSHARE_SUPPLY) / BLOCKS_WITH_REWARD)  
#define REWARD_DELTA_PER_BLOCK        (INITIAL_REWARD / BLOCKS_WITH_REWARD)




#define COINBASE_WAIT_PERIOD          (BLOCKS_PER_HOUR*8) // blocks before a coinbase can be spent
#define DESIRED_PEER_COUNT            (8)                 // number of nodes to connect to
#define BITCHAT_CHANNEL_SIZE          (512*1024*1024)     // 512 MB of history... 
#define BITCHAT_CACHE_WINDOW_SEC      (60*60*24*30)       // 1 month
#define BITCHAT_TARGET_BPS            (128*1024)          // 128 kbit / sec target data rate
#define BITCHAT_BANDWIDTH_WINDOW_US   (5*60*1000*1000ll)  // 5 minutes
#define BITCHAT_INVENTORY_WINDOW_SEC  (60)                // seconds to keep inventory items around
#define DEFAULT_MINING_EFFORT_PERCENT (50)                // percent of CPU to use for mining
#define DEFAULT_MINING_THREADS        (1)                 // number of mining threads to use
#define MIN_NAME_DIFFICULTY           (24)              // number if leeding 0 bits in double sha512 required to register a name
//#define MIN_NAME_DIFFICULTY           (16)                // number if leeding 0 bits in double sha512 required to register a name
#define PEER_HOST_CACHE_QUERY_LIMIT   (1000)              // number of ip/ports that we will cache
#define MAX_CHANNELS_PER_CONNECTION   (32)

// blockchain channel config
#define TRX_INV_QUERY_LIMIT           (2000) // number of trx that may be sent as part of inventory or request msg
#define BLOCK_INV_QUERY_LIMIT         (2000) // number of trx that may be sent as part of inventory or request msg


/**
 *  How much space can be consumed by the trx portion of a block.  This is calculated to
 *  leave room for the block header, proof, and state as well as some room for merged mining
 *  merkle root and future growth without having a full block ever exceed 1MB
 */
#define MAX_BLOCK_TRXS_SIZE           (1024*1024 - 2*sizeof( bts::blockchain::block )  )


#define BLOCKCHAIN_TIMEKEEPER_MIN_BACK_SEC (60*60) // 60 minutes
#define BITNAME_TIME_TOLLERANCE_SEC        (60*60) // 60 minutes
#define BITNAME_BLOCKS_BEFORE_TRANSFER     (288*7) // 1 week before a transfer is complete 
#define BITNAME_BLOCKS_PER_YEAR            (288*365)
