/**
 * @file TCPConnection.h
 * @brief 和Acceptor类是平行关系、十分相似，他们都直接由Server管理，
 *  由一个Channel分发到epoll，通过回调函数处理相应事件。唯一的不同在于，
 *  Acceptor类的处理事件函数（也就是新建连接功能）被放到了Server类中,
 *  而Connection类则没有必要这么做，处理事件的逻辑应该由Connection类本身来完成。
 *
 *  一个高并发服务器一般只会有一个Acceptor用于接受连接（也可以有多个），
 *  但可能会同时拥有成千上万个TCP连接，也就是成千上万个Connection类的实例
 */

#pragma once

#include <LuteBase.h>
#include <polaris/Buffer.h>
#include <polaris/Callbacks.h>
#include <polaris/InetAddress.h>

#include <memory>

// struct tcp_info is in <netinet/tcp.h>
struct tcp_info;

namespace Lute {

class Channel;
class EventLoop;
class Socket;
/// This is an interface class, so don't expose too much details.
/**
 * @brief TCP connection, for both client and server usage.
 *
 */
class TCPConnection : public std::enable_shared_from_this<TCPConnection> {
public:
    // non-copyable
    TCPConnection(const TCPConnection&) = delete;
    TCPConnection operator=(TCPConnection&) = delete;

    /// Constructs a TCPConnection with a connected sockfd
    /// User should not create this object.
    TCPConnection(EventLoop* loop, const std::string& name, int sockfd,
                  const InetAddress& localAddr, const InetAddress& peerAddr);
    ~TCPConnection();

    inline EventLoop* getLoop() const { return loop_; }
    inline const std::string& name() const { return name_; }
    inline const InetAddress& localAddress() const { return localAddr_; }
    inline const InetAddress& peerAddress() const { return peerAddr_; }
    inline bool connected() const { return state_ == StateE::kConnected; }
    inline bool disconnected() const { return state_ == StateE::kDisconnected; }

    // return true if success.
    bool getTcpInfo(struct tcp_info*) const;
    std::string getTcpInfoString() const;

    void send(const void* message, int len);
    // void send(const StringPiece& message);
    /// XXX std::string_view
    void send(const std::string& message);

    /// this one will swap data
    void send(Buffer* buffer);

    // TODO Use std::move to avoid to copy memory
    // void send(string&& message); // C++11
    // void send(Buffer&& message); // C++11

    // NOT thread safe, no simultaneous calling
    void shutdown();
    // void shutdownAndForceCloseAfter(double seconds); // NOT thread
    // safe, no simultaneous calling
    void forceClose();
    void forceCloseWithDelay(double seconds);

    void setTcpNoDelay(bool on);
    // reading or not
    void startRead();
    void stopRead();
    bool isReading() const {
        return reading_;
    };  // NOT thread safe, may race with start/stopReadInLoop

    inline void setContext(const Lute::any& context) { context_ = context; }

    inline const Lute::any& getContext() const { return context_; }

    inline Lute::any* getMutableContext() { return &context_; }

    inline void setConnectionCallback(const ConnectionCallback& cb) {
        connectionCallback_ = cb;
    }

    inline void setMessageCallback(const MessageCallback& cb) {
        messageCallback_ = cb;
    }

    inline void setWriteCompleteCallback(const WriteCompleteCallback& cb) {
        writeCompleteCallback_ = cb;
    }

    inline void setHighWaterMarkCallback(const HighWaterMarkCallback& cb,
                                         size_t highWaterMark) {
        highWaterMarkCallback_ = cb;
        highWaterMark_ = highWaterMark;
    }

    /// Advanced interface
    inline Buffer* inputBuffer() { return &inputBuffer_; }

    inline Buffer* outputBuffer() { return &outputBuffer_; }

    /// Internal use only.
    inline void setCloseCallback(const CloseCallback& cb) {
        closeCallback_ = cb;
    }

    // called when TcpServer accepts a new connection
    void connectEstablished();  // should be called only once
    // called when TcpServer has removed me from its map
    void connectDestroyed();  // should be called only once

private:
    enum class StateE {
        kDisconnected,
        kConnecting,
        kConnected,
        kDisconnecting
    };

    // 事件循环
    EventLoop* loop_;
    const std::string name_;
    StateE state_;  // FIXME: use atomic variable
    bool reading_;

    // we don't expose those classes to client.
    // 客户端的socket fd，每一个Connection对应一个socket fd
    std::unique_ptr<Socket> socket_;
    // 独有的Channel负责分发到epoll，
    // 该Channel的事件处理函数handleEvent()会调用Connection中的事件处理函数来响应客户端请求
    std::unique_ptr<Channel> channel_;
    const InetAddress localAddr_;
    const InetAddress peerAddr_;

    // Callback func
    ConnectionCallback connectionCallback_;
    MessageCallback messageCallback_;
    WriteCompleteCallback writeCompleteCallback_;
    HighWaterMarkCallback highWaterMarkCallback_;
    CloseCallback closeCallback_;

    size_t highWaterMark_;
    Buffer inputBuffer_;
    Buffer outputBuffer_;  // FIXME: use list<Buffer> as output buffer.
    Lute::any context_;
    // FIXME: creationTime_, lastReceiveTime_
    //        bytesReceived_, bytesSent_

private:
    void handleRead(Timestamp receiveTime);
    void handleWrite();
    void handleClose();
    void handleError();

    // void sendInLoop(string&& message);
    // XXX std::string_view
    void sendInLoop(const std::string& message);
    void sendInLoop(const void* message, size_t len);

    void shutdownInLoop();
    // void shutdownAndForceCloseInLoop(double seconds);
    void forceCloseInLoop();
    void setState(StateE s) { state_ = s; }
    const char* stateToString() const;
    void startReadInLoop();
    void stopReadInLoop();
};

/// Define
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;

}  // namespace Lute
