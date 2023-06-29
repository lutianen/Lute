/**
 * @file InetAddress.h
 * @brief
 */

#pragma once

#include <LuteBase.h>
#include <netinet/in.h>

namespace Lute {

namespace sockets {
    const sockaddr* sockaddr_cast(const sockaddr_in6* addr);
}  // namespace sockets

class InetAddress {
private:
    union {
        sockaddr_in addr_;
        sockaddr_in6 addr6_;
    };

public:
    /**
     * Constructs an endpoint with given port number.
     * Mostly used in TcpServer listening.
     */
    explicit InetAddress(uint16_t port = 0, bool loopbackOnly = false,
                         bool ipv6 = false);
    /// Constructors an endpoint with given ip and port.
    /// @c ip should be "1.2.3.4"
    /// XXX std::string_view
    InetAddress(const std::string& ip, uint16_t port, bool ipv6 = false);
    /// Constructs an endpoint with given struct @c sockaddr_in
    /// Mostly used when accepting new connections
    explicit InetAddress(const sockaddr_in& addr) : addr_(addr) {}
    explicit InetAddress(const sockaddr_in6& addr) : addr6_(addr) {}

    /* default copy/assignment are okey */

    inline sa_family_t family() const { return addr_.sin_family; }
    std::string toIp() const;
    std::string toIpPort() const;
    uint16_t toPort() const;

    inline const sockaddr* getSockAddr() const {
        return sockets::sockaddr_cast(&addr6_);
    }
    inline void setSockAddrInet6(const sockaddr_in6& addr6) { addr6_ = addr6; }

    uint32_t ipNetEndian() const;
    inline uint16_t portNetEndian() const { return addr_.sin_port; }

    ///
    /// @brief Resolve hostname to IP address, not changing port or sin_family
    /// @note thread safe
    /// @param hostname
    /// @param result
    /// @return return true on success
    ///
    /// XXX std::string_view
    static bool resolve(const std::string& hostname, InetAddress* result);
    // static std::vector<InetAddress> resolveAll(const char* hostname,
    // uint16_t port = 0);

    // set IPv6 ScopeID
    void setScopeId(uint32_t scope_id);
};

}  // namespace Lute