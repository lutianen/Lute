/**
 * @file TCPClient.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/Connector.h>
#include <polaris/EventLoop.h>
#include <polaris/Sockets.h>
#include <polaris/TCPClient.h>

#include <cstdio>  // snprintf

using namespace Lute;

namespace Lute {

namespace detail {

    void removeConnection(EventLoop* loop, const TCPConnectionPtr& conn) {
        loop->queueInLoop(std::bind(&TCPConnection::connectDestroyed, conn));
    }

    void removeConnector(const ConnectorPtr& connector) {
        // connector->
        // FIXME: removeConnector
        (void)connector;
    }

}  // namespace detail

}  // namespace Lute

// TcpClient::TcpClient(EventLoop* loop)
//   : loop_(loop)
// {
// }

// TcpClient::TcpClient(EventLoop* loop, const string& host, uint16_t port)
//   : loop_(CHECK_NOTNULL(loop)),
//     serverAddr_(host, port)
// {
// }

TCPClient::TCPClient(EventLoop* loop, const InetAddress& serverAddr,
                     const std::string& nameArg)
    : loop_(CHECK_NOTNULL(loop)),
      connector_(new Connector(loop, serverAddr)),
      name_(nameArg),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      retry_(false),
      connect_(true),
      nextConnId_(1) {
    connector_->setNewConnectionCallback(
        std::bind(&TCPClient::newConnection, this, std::placeholders::_1));
    // FIXME setConnectFailedCallback
    LOG_INFO << "TcpClient::TcpClient[" << name_ << "] - connector "
             << get_pointer(connector_);
}

TCPClient::~TCPClient() {
    LOG_INFO << "TcpClient::~TcpClient[" << name_ << "] - connector "
             << get_pointer(connector_);
    TCPConnectionPtr conn;
    bool unique = false;
    {
        MutexLockGuard lock(mutex_);
        unique = connection_.unique();
        conn = connection_;
    }
    if (conn) {
        assert(loop_ == conn->getLoop());
        // FIXME: not 100% safe, if we are in different thread
        CloseCallback cb =
            std::bind(&detail::removeConnection, loop_, std::placeholders::_1);
        loop_->runInLoop(std::bind(&TCPConnection::setCloseCallback, conn, cb));
        if (unique) {
            conn->forceClose();
        }
    } else {
        connector_->stop();
        // FIXME: HACK
        loop_->runAfter(1, std::bind(&detail::removeConnector, connector_));
    }
}

void TCPClient::connect() {
    // FIXME: check state
    LOG_INFO << "TcpClient::connect[" << name_ << "] - connecting to "
             << connector_->serverAddress().toIpPort();
    connect_ = true;
    connector_->start();
}

void TCPClient::disconnect() {
    connect_ = false;

    {
        MutexLockGuard lock(mutex_);
        if (connection_) {
            connection_->shutdown();
        }
    }
}

void TCPClient::stop() {
    connect_ = false;
    connector_->stop();
}

void TCPClient::newConnection(int sockfd) {
    loop_->assertInLoopThread();
    InetAddress peerAddr(sockets::getPeerAddr(sockfd));
    char buf[32];
    snprintf(buf, sizeof buf, ":%s#%d", peerAddr.toIpPort().c_str(),
             nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    InetAddress localAddr(sockets::getLocalAddr(sockfd));
    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TCPConnectionPtr conn(
        new TCPConnection(loop_, connName, sockfd, localAddr, peerAddr));

    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TCPClient::removeConnection, this,
                                     std::placeholders::_1));  // FIXME: unsafe
    {
        MutexLockGuard lock(mutex_);
        connection_ = conn;
    }
    conn->connectEstablished();
}

void TCPClient::removeConnection(const TCPConnectionPtr& conn) {
    loop_->assertInLoopThread();
    assert(loop_ == conn->getLoop());

    {
        MutexLockGuard lock(mutex_);
        assert(connection_ == conn);
        connection_.reset();
    }

    loop_->queueInLoop(std::bind(&TCPConnection::connectDestroyed, conn));
    if (retry_ && connect_) {
        LOG_INFO << "TcpClient::connect[" << name_ << "] - Reconnecting to "
                 << connector_->serverAddress().toIpPort();
        connector_->restart();
    }
}
