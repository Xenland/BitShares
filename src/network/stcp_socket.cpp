#include <bts/network/stcp_socket.hpp>
#include <fc/crypto/hex.hpp>
#include <assert.h>
#include <fc/log/logger.hpp>
#include <fc/network/ip.hpp>
#include <fc/exception/exception.hpp>

namespace bts { namespace network {

stcp_socket::stcp_socket()
:_buf_len(0)
{
}
stcp_socket::~stcp_socket()
{
}

void     stcp_socket::connect_to( const fc::ip::endpoint& ep )
{
    _sock.connect_to( ep );
    _priv_key = fc::ecc::private_key::generate();
    fc::ecc::public_key pub = _priv_key.get_public_key();
    auto s = pub.serialize();
    _sock.write( (char*)&s, sizeof(s) );
    fc::ecc::public_key_data rpub;
    _sock.read( (char*)&rpub, sizeof(rpub) );

    auto shared_secret = _priv_key.get_shared_secret( rpub );
//    ilog("shared secret ${s}", ("s", shared_secret) );
    _send_bf.start( (unsigned char*)&shared_secret, 54 );
    _recv_bf.start( (unsigned char*)&shared_secret, 54 );
}

/**
 *   This method must read at least 8 bytes at a time from
 *   the underlying TCP socket so that it can decrypt them. It
 *   will buffer any left-over.
 */
size_t   stcp_socket::readsome( char* buffer, size_t max )
{
    assert( (max % 8) == 0 );
    assert( max >= 8 );

    size_t s = _sock.readsome( buffer, max );
    if( s < 8 ) 
    {
        _sock.read( buffer + s, 8 - s );
        s = 8;
    }
    //_recv_bf.decrypt( (unsigned char*)buffer, s );
    return s;
}

bool     stcp_socket::eof()const
{
  return _sock.eof();
}

size_t   stcp_socket::writesome( const char* buffer, size_t len )
{
    assert( len % 8 == 0 );
    assert( len > 0 );
    unsigned char crypt_buf[2048];
    len = std::min<size_t>(sizeof(crypt_buf),len);
    memcpy( crypt_buf, buffer, len );
    /**
     * every sizeof(crypt_buf) bytes the blowfish channel
     * has an error and doesn't decrypt properly...  disable
     * for now because we are going to upgrade to something
     * better.
     */
    //_send_bf.encrypt( (unsigned char*)crypt_buf, len );
    _sock.write( (char*)buffer, len );
    return len;
}

void     stcp_socket::flush()
{
   _sock.flush();
}


void     stcp_socket::close()
{
  try 
  {
   _sock.close();
  }FC_RETHROW_EXCEPTIONS( warn, "error closing stcp socket" );
}

void    stcp_socket::accept()
{
    _priv_key = fc::ecc::private_key::generate();
    fc::ecc::public_key pub = _priv_key.get_public_key();
    auto s = pub.serialize();
    _sock.write( (char*)&s, sizeof(s) );
    fc::ecc::public_key_data rpub;
    _sock.read( (char*)&rpub, sizeof(rpub) );

    auto shared_secret = _priv_key.get_shared_secret( fc::ecc::public_key(rpub) );
//    ilog("shared secret ${s}", ("s", shared_secret) );
    _send_bf.start( (unsigned char*)&shared_secret, 54 );
    _recv_bf.start( (unsigned char*)&shared_secret, 54 );
}


}} // namespace bts::network
