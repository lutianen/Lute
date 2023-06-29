/**
 * @file Acceptor.h
 * @brief 用于接受 TCP 连接.
 *  对于每一个事件，不管提供什么样的服务，首先需要做的事都是调用 `accept` 函数
 *  接受 TCP 连接，然后将 `socket` 文件描述符添加到 `epoll`.
 *  当这个 I/O 有事件发生时，再对此 TCP 连接提供相应的服务.
 */

#pragma once

#include <polaris/Channel.h>
#include <polaris/Sockets.h>

namespace Lute {

class EventLoop;
class InetAddress;
/// Acceptor of incoming TCP connections.
/// 用于接受TCP连接,它是 TcpServer 的成员,生命期由后者控制
class Acceptor {
public:
    using NewConnectionCallback =
        std::function<void(int sockfd, const InetAddress&)>;

private:
    // Reactor 模式中的 main-Reactor
    EventLoop* loop_;
    //
    Socket acceptSocket_;
    // 负责分发到 epoll, 该 `Channel` 的事件处理函数 ``,
    // 调用 `Acceptor` 中的接受新连接函数来新建一个 TCP 连接
    Channel acceptChannel_;
    /**
     * @brief 新建连接回调函数 * 在 `TcpServer` 类中实现 *
     * FIXME Acceptor的Channel使用了ET模式，事实上使用LT模式更合适
     * @param connfd int
     * @param peerAddr InetAddress
     */
    NewConnectionCallback newConnectionCallback_;

    bool listenning_;
    int idleFd_;

    void handleRead();

public:
    // noncopyable
    Acceptor(const Acceptor&) = delete;
    Acceptor& operator=(Acceptor&) = delete;

    Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reuseport);
    ~Acceptor();

    inline void setNewConnectionCallback(const NewConnectionCallback& cb) {
        newConnectionCallback_ = cb;
    }

    inline bool listenning() const { return listenning_; }
    void listen();
};

}  // namespace Lute
