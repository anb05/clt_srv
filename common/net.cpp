#include "./net.hpp"

#include <stdexcept>

namespace net {
void sleepFor(uint64_t mSec)
{
    std::this_thread::sleep_for(std::chrono::milliseconds(mSec));
}

Address::Address(uint32_t address, uint16_t port)
    : m_address {address}
    , m_port    {port}
{ }

Address::Address(uint8_t a, uint8_t b, uint8_t c, uint8_t d, uint16_t port)
{
    m_address = ( a << 24 ) | ( b << 16 ) | ( c << 8 ) | d;
    m_port = port;
}

Socket::Socket(uint16_t port)
    : m_socket {0}
{
    if (0 == port) {
        return;
    }

    if(!open(port)) {
        throw (std::domain_error("failed to create socket\n"));
    }
}

bool Socket::open(uint16_t port)
{
    assert( !isOpen() );

    // create socket
    m_socket = ::socket( AF_INET, SOCK_DGRAM, IPPROTO_UDP );

    if ( m_socket <= 0 ) {
        printf( "failed to create socket\n" );
        m_socket = 0;
        return false;
    }

    // bind to port
    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( port );

    if ( bind(m_socket, (const sockaddr*) &address, sizeof(sockaddr_in)) < 0) {
        printf( "failed to bind socket\n" );
        closeSocket();
        return false;
    }

    // set non-blocking io

#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

    int nonBlocking = 1;
    if ( fcntl( m_socket, F_SETFL, O_NONBLOCK, nonBlocking ) == -1 ) {
        printf( "failed to set non-blocking socket\n" );
        closeSocket();
        return false;
    }

#elif PLATFORM == PLATFORM_WINDOWS

    DWORD nonBlocking = 1;
    if ( ioctlsocket( m_socket, FIONBIO, &nonBlocking ) != 0 ) {
        printf( "failed to set non-blocking socket\n" );
        closeSocket();
        return false;
    }

#endif

    return true;
}

void Socket::closeSocket()
{
    if ( m_socket != 0 ) {
#if PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX
        close( m_socket );
#elif PLATFORM == PLATFORM_WINDOWS
        closesocket( socket );
#endif
        m_socket = 0;
    }
}

bool Socket::send(const Address& destination, const void* data, int32_t size)
{
    assert( data );
    assert( size > 0 );

    if ( m_socket == 0 ) {
        return false;
    }

    sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl( destination.address() );
    address.sin_port = htons( destination.port() );

    int32_t sent_bytes = sendto( m_socket, (const char*)data, size, 0, (sockaddr*)&address, sizeof(sockaddr_in) );

    return sent_bytes == size;
}

int32_t Socket::receive(Address& sender, void* data, int32_t size)
{
    assert( data );
    assert( size > 0 );

    if ( m_socket == 0 )
        return false;

#if PLATFORM == PLATFORM_WINDOWS
    typedef int socklen_t;
#endif

    sockaddr_in from;
    socklen_t fromLength = sizeof( from );

    int32_t received_bytes = recvfrom( m_socket, (char*)data, size, 0, (sockaddr*)&from, &fromLength );

    if ( received_bytes <= 0 )
        return 0;

    uint32_t address = ntohl( from.sin_addr.s_addr );
    uint16_t port = ntohs( from.sin_port );

    sender = Address( address, port );

    return received_bytes;
}

} // namespace net
