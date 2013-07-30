#pragma once

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
#define DEFAULT_SERVER_PORT           (9876)
#define DESIRED_PEER_COUNT            (8)                 // number of nodes to connect to
#define BITCHAT_TARGET_BPS            (128*1024)          // 128 kbit / sec target data rate
#define BITCHAT_BANDWIDTH_WINDOW_US   (5*60*1000*1000ll)  // 5 minutes
#define BITCHAT_INVENTORY_WINDOW_SEC  (60)                // seconds to keep inventory items around
#define DEFAULT_MINING_EFFORT_PERCENT (50)                // percent of CPU to use for mining
#define DEFAULT_MINING_THREADS        (4)                 // number of mining threads to use
#define MIN_NAME_DIFFICULTY           (24)                // number if leeding 0 bits in double sha512 required to register a name
#define PEER_HOST_CACHE_QUERY_LIMIT   (1000)              // number of ip/ports that we will cache
#define MAX_CHANNELS_PER_CONNECTION   (32)
