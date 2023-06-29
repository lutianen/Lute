///
/// @file Sockets.cc
/// @brief
///

#include <LuteBase.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <polaris/Sockets.h>
#include <sys/socket.h>
#include <sys/uio.h>
#include <unistd.h>

#include <cstdio>

using namespace Lute;

namespace {
using SA = struct sockaddr;

/// Accept4 函数可以在接受连接时，设置为非阻塞模式和关闭“close-on-exec”标志
#if VALGRIND || defined(NO_ACCEPT4)
void setNonBlockAndCloseOnExec(int sockfd) {
    // non-block
    int flags = ::fcntl(sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    int ret = ::fcntl(sockfd, F_SETFL, flags);
    if (ret < 0) LOG_SYSERR << "setNonBlock Failed.";

    // close-on-exec
    flags = ::fcntl(sockfd, F_GETFD, 0);
    flags |= FD_CLOEXEC;
    ret = ::fcntl(sockfd, F_SETFD, flags);
    if (ret < 0) LOG_SYSERR << "setCloseOnExec Failed.";
}
#endif
}  // namespace

const sockaddr* sockets::sockaddr_cast(const sockaddr_in6* addr) {
    return reinterpret_cast<const sockaddr*>(
        reinterpret_cast<const void*>(addr));
}
sockaddr* sockets::sockaddr_cast(sockaddr_in6* addr) {
    return reinterpret_cast<sockaddr*>(reinterpret_cast<void*>(addr));
}
const sockaddr* sockets::sockaddr_cast(const sockaddr_in* addr) {
    return reinterpret_cast<const sockaddr*>(
        reinterpret_cast<const void*>(addr));
}
const sockaddr_in* sockets::sockaddr_in_cast(const sockaddr* addr) {
    return reinterpret_cast<const sockaddr_in*>(
        reinterpret_cast<const void*>(addr));
}
const sockaddr_in6* sockets::sockaddr_in6_cast(const sockaddr* addr) {
    return reinterpret_cast<const sockaddr_in6*>(
        reinterpret_cast<const void*>(addr));
}

/**
 * @brief Create a TCP Non-blocking socket
 *
 * @param family AF_UNIX / AF_INET / AF_INET6 / ...
 * @return int
 */
int sockets::createNonblockingOrDie(sa_family_t family) {
#if VALGRIND
    int sockfd = ::socket(family, SOCK_STREAM, IPPROTO_TCP);
    if (sockfd < 0) LOG_SYSFATAL << "sockets::createNonblockingOrDie";
    setNonBlockAndCloseOnExec(sockfd);
#else
    int sockfd = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC,
                          IPPROTO_TCP);
    if (sockfd < 0) LOG_SYSFATAL << "sockets::createNonblockingOrDie";
#endif
    return sockfd;
}

///
/// @brief Bind addr to sockfd, or die
///
void sockets::bindOrDie(int sockfd, const sockaddr* addr) {
    int ret =
        ::bind(sockfd, addr, static_cast<socklen_t>(sizeof(sockaddr_in6)));
    if (ret < 0) LOG_SYSFATAL << "sockets::bindOrDie";
}

///
/// @brief Listen sockfd, or die
/// @note SOMAXCONN
///
void sockets::listenOrDie(int sockfd) {
    int ret = ::listen(sockfd, SOMAXCONN);
    if (ret < 0) LOG_SYSFATAL << "sockets::listenOrDie";
}

///
/// @brief Accept a new connection and return connfd
/// @param sockfd 监听 fd
/// @param addr 客户端地址
/// @return int confd or -1 for error
///
int sockets::accept(int sockfd, sockaddr_in6* addr) {
    socklen_t addrlen = static_cast<socklen_t>(sizeof *addr);

#if VALGRIND || defined(NO_ACCEPT4)
    int connfd = ::accept(sockfd, sockaddr_cast(addr), &addrlen);
    setNonBlockAndCloseOnExec(connfd);
#else
    int connfd = ::accept4(sockfd, sockaddr_cast(addr), &addrlen,
                           SOCK_NONBLOCK | SOCK_CLOEXEC);
#endif

    if (connfd < 0) {
        int savedErrno = errno;
        LOG_SYSERR << "Socket::accept";
        switch (savedErrno) {
            case EAGAIN:
            case ECONNABORTED:
            case EINTR:
            case EPROTO:  // ???
            case EPERM:
            case EMFILE:  // per-process lmit of open file desctiptor ???
                // expected errors
                errno = savedErrno;
                break;
            case EBADF:
            case EFAULT:
            case EINVAL:
            case ENFILE:
            case ENOBUFS:
            case ENOMEM:
            case ENOTSOCK:
            case EOPNOTSUPP:
                // unexpected errors
                LOG_FATAL << "unexpected error of ::accept " << savedErrno;
                break;
            default:
                LOG_FATAL << "unknown error of ::accept " << savedErrno;
                break;
        }
    }
    return connfd;
}

///
/// Open a connection on socket SOCKETFD to peer at ADDR (which LEN bytes long).
/// @param sockfd 由 socket 系统调用返回一个 socket
/// @param addr 服务器监听的 socket 地址
/// @return Return 0 on success, -1 for errors.
///  常见的两种 errno : \n
///      - ECONNREFUSED 目标端口不存在，连接拒绝 \n
///      - ETIMEOUT 连接超时
///
int sockets::connect(int sockfd, const sockaddr* addr) {
    return ::connect(sockfd, addr,
                     static_cast<socklen_t>(sizeof(sockaddr_in6)));
}

///
/// @brief Read NBYTES into BUF from FD.
/// @return Return the number read, -1 for errors or 0 for EOF.
///
ssize_t sockets::read(int sockfd, void* buf, size_t count) {
    return ::read(sockfd, buf, count);
}

///
/// @brief 用于从指定文件描述符 sockfd 中读取数据,
///         支持一次读取多个非连续缓冲区（散布读取）
/// @param sockfd 待读取的文件描述符
/// @param iov 指向一个或多个缓冲区的指针数组（描述缓冲区地址的和长度）
/// @param iovcnt 缓冲区的数量
/// @return ssize_t
///         > 0 - 实际读取的字节数;
///         = 0 - 文件末尾;
///         < 0 - 出错
///
ssize_t sockets::readv(int sockfd, const iovec* iov, int iovcnt) {
    return ::readv(sockfd, iov, iovcnt);
}

/**
 * @brief Write N bytes of BUF to FD.
 * @return Return the number written, or -1.
 */
ssize_t sockets::write(int sockfd, const void* buf, size_t count) {
    return ::write(sockfd, buf, count);
}

/**
 * @brief Close the file descriptor SOCKFD.
 * close 系统调用并非总是立即关闭一个连接，而是将 fd 的引用计数减 1 ，
 * 只有当 fd 的引用计数为 0 时，才真正关闭连接.
 */
void sockets::close(int sockfd) {
    /// returns  zero on success.
    /// On error, -1 is returned, and errno is set to indicate the error.
    if (::close(sockfd) < 0) LOG_SYSERR << "sockets::close";
}

/**
 * @brief Shut down all or part of the connection open on socket FD. \n
 * 关闭方式：SHUT_WR. 这种情况下，连接处于半关闭状态. \n
 * 关闭 sockfd 上写的这一半，sockfd 的发送缓冲区中的数据会在真正关闭连接
 * 之前全部发送出去，应用程序不可再对该 socket 文件描述符执行写操作.
 */
void sockets::shutdownWrite(int sockfd) {
    /// SHUT_RD   = No more receptions;
    /// SHUT_WR   = No more transmissions;
    /// SHUT_RDWR = No more receptions or transmissions.
    /// Returns 0 on success, -1 for errors.
    if (::shutdown(sockfd, SHUT_WR) < 0) LOG_SYSERR << "sockets::shutdownWrite";
}

void sockets::toIpPort(char* buf, size_t size, const sockaddr* addr) {
    toIp(buf, size, addr);
    size_t end = ::strlen(buf);
    const sockaddr_in* addr4 = sockaddr_in_cast(addr);
    uint16_t port = sockets::networkToHost16(addr4->sin_port);
    assert(size > end);
    snprintf(buf + end, size - end, ":%u", port);
}

void sockets::toIp(char* buf, size_t size, const sockaddr* addr) {
    if (addr->sa_family == AF_INET) {
        assert(size >= INET_ADDRSTRLEN);
        const sockaddr_in* addr4 = sockaddr_in_cast(addr);
        ::inet_ntop(AF_INET, &addr4->sin_addr, buf,
                    static_cast<socklen_t>(size));
    } else if (addr->sa_family == AF_INET6) {
        assert(size >= INET6_ADDRSTRLEN);
        const sockaddr_in6* addr6 = sockaddr_in6_cast(addr);
        ::inet_ntop(AF_INET6, &addr6->sin6_addr, buf,
                    static_cast<socklen_t>(size));
    }
}

void sockets::fromIpPort(const char* ip, uint16_t port, sockaddr_in* addr) {
    addr->sin_family = AF_INET;
    addr->sin_port = hostToNetwork16(port);

    if (::inet_pton(AF_INET, ip, &addr->sin_addr) <= 0)
        LOG_SYSERR << "sockets::fromIpPort";
}

void sockets::fromIpPort(const char* ip, uint16_t port, sockaddr_in6* addr) {
    addr->sin6_family = AF_INET6;
    addr->sin6_port = hostToNetwork16(port);

    if (::inet_pton(AF_INET6, ip, &addr->sin6_addr) <= 0)
        LOG_SYSERR << "sockets::fromIpPort";
}

int sockets::getSocketError(int sockfd) {
    int optval;
    socklen_t optlen = static_cast<socklen_t>(sizeof optval);

    if (::getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
        return errno;
    } else {
        return optval;
    }
}

sockaddr_in6 sockets::getLocalAddr(int sockfd) {
    sockaddr_in6 localaddr;
    memZero(&localaddr, sizeof(localaddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(localaddr));

    if (::getsockname(sockfd, sockaddr_cast(&localaddr), &addrlen) < 0)
        LOG_SYSERR << "sockets::getLocalAddr";

    return localaddr;
}

sockaddr_in6 sockets::getPeerAddr(int sockfd) {
    sockaddr_in6 peeraddr;
    memZero(&peeraddr, sizeof(peeraddr));
    socklen_t addrlen = static_cast<socklen_t>(sizeof(peeraddr));

    if (::getpeername(sockfd, sockaddr_cast(&peeraddr), &addrlen) < 0)
        LOG_SYSERR << "sockets::getPeerAddr";

    return peeraddr;
}

bool sockets::isSelfConnect(int sockfd) {
    sockaddr_in6 localaddr = getLocalAddr(sockfd);
    sockaddr_in6 peeraddr = getPeerAddr(sockfd);
    if (localaddr.sin6_family == AF_INET) {
        const sockaddr_in* laddr4 =
            reinterpret_cast<struct sockaddr_in*>(&localaddr);
        const sockaddr_in* raddr4 = reinterpret_cast<sockaddr_in*>(&peeraddr);

        return laddr4->sin_port == raddr4->sin_port &&
               laddr4->sin_addr.s_addr == raddr4->sin_addr.s_addr;
    } else if (localaddr.sin6_family == AF_INET6) {
        return localaddr.sin6_port == peeraddr.sin6_port &&
               memcmp(&localaddr.sin6_addr, &peeraddr.sin6_addr,
                      sizeof(localaddr.sin6_addr)) == 0;
    } else {
        return false;
    }
}

// -----------------------------------
using namespace Lute;

Socket::~Socket() { sockets::close(sockfd_); }

bool Socket::getTcpInfo(struct tcp_info* tcpInfo) const {
    socklen_t len = sizeof(*tcpInfo);
    memZero(tcpInfo, len);
    return ::getsockopt(sockfd_, SOL_TCP, TCP_INFO, tcpInfo, &len) == 0;
}

bool Socket::getTcpInfoString(char* buf, int len) const {
    tcp_info tcpi{};
    bool ok = getTcpInfo(&tcpi);
    if (ok) {
        snprintf(buf, static_cast<size_t>(len),
                 "unrecovered=%u "
                 "rto=%u ato=%u snd_mss=%u rcv_mss=%u "
                 "lost=%u retrans=%u rtt=%u rttvar=%u "
                 "sshthresh=%u cwnd=%u total_retrans=%u",
                 tcpi.tcpi_retransmits,  // Number of unrecovered [RTO] timeouts
                 tcpi.tcpi_rto,          // Retransmit timeout in usec
                 tcpi.tcpi_ato,          // Predicted tick of soft clock in usec
                 tcpi.tcpi_snd_mss, tcpi.tcpi_rcv_mss,
                 tcpi.tcpi_lost,     // Lost packets
                 tcpi.tcpi_retrans,  // Retransmitted packets out
                 tcpi.tcpi_rtt,      // Smoothed round trip time in usec
                 tcpi.tcpi_rttvar,   // Medium deviation
                 tcpi.tcpi_snd_ssthresh, tcpi.tcpi_snd_cwnd,
                 tcpi.tcpi_total_retrans);  // Total retransmits for entire
                                            // connection
    }

    return ok;
}

void Socket::bindAddress(const InetAddress& localaddr) {
    sockets::bindOrDie(sockfd_, localaddr.getSockAddr());
}

void Socket::listen() { sockets::listenOrDie(sockfd_); }

int Socket::accept(InetAddress* peeraddr) {
    sockaddr_in6 addr{};
    memZero(&addr, sizeof(addr));

    int connfd = sockets::accept(sockfd_, &addr);
    if (connfd >= 0) {
        peeraddr->setSockAddrInet6(addr);
    }

    return connfd;
}

void Socket::shutdownWrite() { sockets::shutdownWrite(sockfd_); }

/**
 * @brief 接收方得满足（不通告小窗口给发送方） + 发送方开启 Nagle
 * 算法，就可以避免糊涂窗口综合症. Nagle
 * 算法默认开启，然而对于一些需要小数据包交互的场景的程序，则需要关闭 Nagle
 * 算法. Solution: 在 Socket 设置 @c TCP_NODELAY 选项关闭 Nagle 算法.
 */
void Socket::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;

    // Upon successful completion, setsockopt() shall return  0.  Otherwise,  -1
    //    shall be returned and errno set to indicate the error.
    int ret = ::setsockopt(sockfd_, IPPROTO_TCP, TCP_NODELAY, &optval,
                           static_cast<socklen_t>(sizeof optval));

    if (0 == ret) {
        LOG_INFO << (on ? "Open TCP_NODELAY ok." : "Close TCP_NODELAY ok.");
    } else {
        LOG_ERROR << "set TCP_NODELAY failed. errno: " << errno;
    }
}

void Socket::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;

    // Upon successful completion, setsockopt() shall return  0.  Otherwise,  -1
    //    shall be returned and errno set to indicate the error.
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEADDR, &optval,
                           static_cast<socklen_t>(sizeof optval));

    if (0 == ret) {
        LOG_INFO << (on ? "Open SO_REUSEADDR ok." : "Close SO_REUSEADDR ok.");
    } else {
        LOG_ERROR << "set SO_REUSEADDR failed. errno: " << errno;
    }
}

void Socket::setReusePort(bool on) {
#ifdef SO_REUSEPORT
    int optval = on ? 1 : 0;
    // Upon successful completion, setsockopt() shall return  0.  Otherwise,  -1
    //    shall be returned and errno set to indicate the error.
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_REUSEPORT, &optval,
                           static_cast<socklen_t>(sizeof optval));

    if (ret < 0 && on) {
        LOG_SYSERR << "SO_REUSEPORT failed.";
    } else {
        LOG_INFO << (on ? "Open SO_REUSEPORT ok." : "Close SO_REUSEPORT ok.");
    }
#else
    if (on) {
        LOG_ERROR << "SO_REUSEPORT is not supported.";
    }
#endif
}

void Socket::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    // Upon successful completion, setsockopt() shall return  0.  Otherwise,  -1
    //    shall be returned and errno set to indicate the error.
    int ret = ::setsockopt(sockfd_, SOL_SOCKET, SO_KEEPALIVE, &optval,
                           static_cast<socklen_t>(sizeof optval));
    if (0 == ret) {
        LOG_INFO << (on ? "Open SO_KEEPALIVE ok." : "Close SO_KEEPALIVE ok.");
    } else {
        LOG_ERROR << "set SO_KEEPALIVE failed. errno: " << errno;
    }
}
