#ifndef NET_HPP
#define NET_HPP

// platform detection

#define PLATFORM_WINDOWS  1
#define PLATFORM_MAC      2
#define PLATFORM_UNIX     3

#if defined(_WIN32)
#define PLATFORM PLATFORM_WINDOWS
#elif defined(__APPLE__)
#define PLATFORM PLATFORM_MAC
#else
#define PLATFORM PLATFORM_UNIX
#endif

#if PLATFORM == PLATFORM_WINDOWS

    #include <winsock2.h>
    #pragma comment( lib, "wsock32.lib" )

#elif PLATFORM == PLATFORM_MAC || PLATFORM == PLATFORM_UNIX

    #include <sys/socket.h>
    #include <sys/types.h>
    #include <netinet/in.h>
    #include <fcntl.h>
    #include <unistd.h>

#else

    #error unknown platform!

#endif

#include <assert.h>
#include <cstdint>
#include <thread>

namespace net
{
/**
 * @brief sleepFor
 * platform independent wait for n seconds
 *
 * @param[in] mSec
 */
void sleepFor(uint64_t mSec);

/**
 * @brief The Address class
 */
class Address
{
public:
    explicit Address( uint32_t address = 0, uint16_t port = 0 );
    explicit Address( unsigned char a, unsigned char b, unsigned char c, unsigned char d, unsigned short port );

    explicit Address(const Address& ) = default;
    explicit Address(Address&& )      = default;

    ~Address() = default;

    Address& operator= (const Address& ) = default;
    Address& operator= (Address&& )      = default;

    uint32_t address() const { return m_address; }

    uint8_t a() const { return (0xff & (m_address >> 24)); }

    uint8_t b() const { return (0xff & (m_address >> 16)); }

    uint8_t c() const { return (0xff & (m_address >> 8)); }

    uint8_t d() const { return (0xff & m_address) ; }

    uint16_t port() const { return m_port; }

    bool operator == (const Address& rhs) const
    {
        return ((m_address == rhs.m_address) && (m_port == rhs.m_port));
    }

    bool operator != (const Address& rhs) const { return !(*this == rhs); }

private:

    uint32_t m_address {0};
    uint16_t m_port    {0};
};

// sockets

inline bool InitializeSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSADATA WsaData;
    return WSAStartup( MAKEWORD(2,2), &WsaData ) != NO_ERROR;
#else
    return true;
#endif
}

inline void ShutdownSockets()
{
#if PLATFORM == PLATFORM_WINDOWS
    WSACleanup();
#endif
}

/**
 * @brief The Socket class
 */
class Socket
{
public:

    explicit Socket(uint16_t port = 0);
    explicit Socket(const Socket& ) = default;
    explicit Socket(Socket&& )      = default;

    Socket& operator= (const Socket& ) = default;
    Socket& operator= (Socket&& )      = default;

    ~Socket() { closeSocket(); }

    bool open( uint16_t port );

    void closeSocket();

    bool isOpen() const { return m_socket != 0; }

    bool send( const Address& destination, const void* data, int32_t size );

    int32_t receive( Address& sender, void* data, int32_t size );

private:
    int32_t m_socket {0};
};

} // namespace net

#endif // NET_HPP
