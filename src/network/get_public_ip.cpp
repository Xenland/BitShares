#include <fc/network/ip.hpp>
#include <fc/network/http/connection.hpp>
#include <fc/log/logger.hpp>

namespace bts { namespace network 
{
  /**
   *  Attempts to connect to checkip.dyndns.org to and then parse the
   *  HTML for the IP address.
   *
   *  @throw if no IP could be discovered. 
   */
  fc::ip::address get_public_ip()
  {
     fc::http::connection con;
     con.connect_to( fc::ip::endpoint::from_string( "91.198.22.70:80" ) );
     auto reply = con.request( "GET", "/", std::string() );

     std::string s( reply.body.data(), reply.body.data() + reply.body.size() );
//     ilog( "${body}", ( "body", s) );
     auto pos = s.find( "Address: " );
     auto end = s.find( "</body>" );

     return fc::ip::address( s.substr( pos + 9, end-pos - 9) );
  }

} } 
