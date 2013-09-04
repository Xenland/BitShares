#include <bts/bitchat/bitchat_message_db.hpp>
#include <fc/reflect/variant.hpp>
#include <fc/exception/exception.hpp>
#include <bts/db/level_pod_map.hpp>






namespace bts { namespace bitchat {
  
  bool operator < ( const message_header& a, const message_header& b )
  {
     if( a.type.value > b.type.value ) return false;
     if( a.type.value < b.type.value ) return true;
     if( a.received_time > b.received_time ) return false;
     if( a.received_time < b.received_time ) return true;
     if( a.to_key > b.to_key ) return false;
     if( a.to_key < b.to_key ) return true;
     if( a.from_key > b.from_key ) return false;
     return true;
  }

  bool operator == ( const message_header& a, const message_header& b )
  {
     return a.type == b.type &&
            a.received_time == b.received_time &&
            a.to_key == b.to_key &&
            a.from_key == b.from_key &&
            a.digest == b.digest;
            // TODO: compare the rest of it...
  }
  namespace detail 
  {
     class message_db_impl
     {
       public:
          db::level_pod_map<message_header,uint32_t>    _index;
          db::level_pod_map<fc::uint256,std::vector<char> > _digest_to_data;
     };

  } // namespace detail

  message_db::message_db()
  :my( new detail::message_db_impl() )
  {
  }

  message_db::~message_db(){}

  void message_db::open( const fc::path& dbdir, const fc::uint512& key, bool create )
  { try {
        fc::create_directories(dbdir);
        my->_index.open(dbdir/"index");
        my->_digest_to_data.open(dbdir/"digest_to_data");
  } FC_RETHROW_EXCEPTIONS( warn, "", ("dir", dbdir)("key",key)("create",create)) }

  void message_db::store( const decrypted_message& m )
  { try {
      FC_ASSERT( m.from_sig    );
      FC_ASSERT( m.from_key    );
      FC_ASSERT( m.decrypt_key );

      message_header head;
      head.type             = m.msg_type;
      head.received_time    = fc::time_point::now();
      head.to_key           = m.decrypt_key->get_public_key();
      head.from_key         = *m.from_key;
      head.digest           = m.digest();
      head.from_sig_time    = m.sig_time;
      head.from_sig         = *m.from_sig;
      my->_index.store(head,0);

      // TODO: consider using city128 rather than 256 to reduce index size
      my->_digest_to_data.store( head.digest, m.data );
  } FC_RETHROW_EXCEPTIONS( warn, "", ("msg",m) ) }
 

  std::vector<message_header>  message_db::fetch_headers( private_message_type t, 
                                              fc::time_point_sec from_time, 
                                              fc::time_point_sec to_time,
                                              fc::optional<fc::ecc::public_key_data> to_key,
                                              fc::optional<fc::ecc::public_key_data> from_key )
  { try {
     std::vector<message_header> headers;
     message_header start;
     start.type = t;
     start.received_time = from_time;
     if( to_key ) start.to_key = *to_key;
     auto itr = my->_index.lower_bound(start);

     while( itr.valid() )
     {
        auto cur_val = itr.key();
        if( cur_val.received_time >= from_time )
        {
            if( cur_val.received_time <= to_time )
            {
               if( to_key && *to_key != cur_val.to_key )
               {
                   ++itr;
                   continue;
               }
               headers.push_back(cur_val);
            }
            else
            {
               return headers;
            }
        }
        else
        {
           return headers;
        }
        ++itr;
     }

     return headers;
  } FC_RETHROW_EXCEPTIONS( warn, "", ("type",t)("from_time",from_time)("to_time",to_time)("to_key",to_key)("from_key",from_key) ) }
  
  std::vector<char> message_db::fetch_data(  const fc::uint256& digest )
  { try {
     std::vector<char> data;
     return data;
  } FC_RETHROW_EXCEPTIONS( warn, "", ("digest",digest ) ) }


} } // bts::bitchat
