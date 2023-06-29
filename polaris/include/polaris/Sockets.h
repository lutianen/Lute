///
/// @file Sockets.h
/// @brief
///

#pragma once

#include <arpa/inet.h>
#include <polaris/InetAddress.h>

#include <utility>

// struct tcp_info in <netinet/tcp.h>
struct tcp_info;

namespace Lute {
namespace sockets {
/// --------------- Endian start ---------------------
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
    /// @brief 64字节主机转网络
    inline uint64_t hostToNetwork64(uint64_t host64) { return htobe64(host64); }
    /// @brief 32字节主机转网络
    inline uint32_t hostToNetwork32(uint32_t host32) { return htobe32(host32); }
    /// @brief 16字节主机转网络
    inline uint16_t hostToNetwork16(uint16_t host16) { return htobe16(host16); }

    /// @brief 64字节网络转主机
    inline uint64_t networkToHost64(uint64_t net64) { return be64toh(net64); }
    /// @brief 32字节网络转主机
    inline uint32_t networkToHost32(uint32_t net32) { return be32toh(net32); }
    /// @brief 16字节网络转主机
    inline uint16_t networkToHost16(uint16_t net16) { return be16toh(net16); }
#pragma GCC diagnostic pop

    /// @brief Creates a non-blocking socket file descriptor, abort if
    /// any eror.
    /// @return socket file descriptor
    int createNonblockingOrDie(sa_family_t family);

    int connect(int sockfd, const sockaddr* addr);
    void bindOrDie(int sockfd, const sockaddr* addr);
    void listenOrDie(int sockfd);
    int accept(int sockfd, sockaddr_in6* addr);

    ssize_t read(int sockfd, void* buf, size_t count);
    ssize_t readv(int sockfd, const iovec* iov, int iovcnt);
    ssize_t write(int sockfd, const void* buf, size_t count);

    void close(int sockfd);
    void shutdownWrite(int sockfd);

    void toIpPort(char* buf, size_t size, const sockaddr* addr);
    void toIp(char* buf, size_t size, const sockaddr* addr);

    void fromIpPort(const char* ip, uint16_t port, sockaddr_in* addr);
    void fromIpPort(const char* ip, uint16_t port, sockaddr_in6* addr);

    int getSocketError(int sockfd);

    const sockaddr* sockaddr_cast(const sockaddr_in* addr);
    const sockaddr* sockaddr_cast(const sockaddr_in6* addr);
    sockaddr* sockaddr_cast(sockaddr_in6* addr);
    const sockaddr_in* sockaddr_in_cast(const sockaddr* addr);
    const sockaddr_in6* sockaddr_in6_cast(const sockaddr* addr);

    sockaddr_in6 getLocalAddr(int sockfd);
    sockaddr_in6 getPeerAddr(int sockfd);
    bool isSelfConnect(int sockfd);
}  // namespace sockets

/// ---------------------------------------------------------

///
/// @brief RAII - fileDescriptor, close it in Dtor.
///
class Socket {
public:
    // noncopyable
    Socket(const Socket&) = delete;
    Socket& operator=(Socket&) = delete;

    explicit Socket(int sockfd) : sockfd_(sockfd) {}
    // TODO Socket(Socket&&) // move constructor in C++11
    Socket(Socket&& that) : sockfd_(std::move(that.sockfd_)) {}
    ~Socket();

    /// Get sockfd
    inline int fd() const { return sockfd_; }

    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    bool getTcpInfoString(char* buf, int len) const;

    /// abort if address in use
    void bindAddress(const InetAddress& localaddr);

    /// abort if address in use
    void listen();

    /// On success, returns a non-negative integer that is
    /// a descriptor for the accepted socket, which has been
    /// set to non-blocking and close-on-exec. *peeraddr is assigned.
    /// On error, -1 is returned, and *peeraddr is untouched.
    int accept(InetAddress* peeraddr);

    void shutdownWrite();

    /// Enable/disable TCP_NODELAY (disable/enable Nagle's algorithm).
    void setTcpNoDelay(bool on);

    /// Enable/disable SO_REUSEADDR
    void setReuseAddr(bool on);

    /// Enable/disable SO_REUSEPORT
    void setReusePort(bool on);

    /// Enable/disable SO_KEEPALIVE
    void setKeepAlive(bool on);

private:
    const int sockfd_;  // socket file descriptor
};
}  // namespace Lute
