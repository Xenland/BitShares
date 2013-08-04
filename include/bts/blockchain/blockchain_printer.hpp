#pragma once
#include <bts/blockchain/blockchain_db.hpp>

namespace bts { namespace blockchain {

  std::string pretty_print( const trx_block& b, blockchain_db& db );

} }
