/**
 * @file TCPServer.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/Acceptor.h>
#include <polaris/Callbacks.h>
#include <polaris/EventLoop.h>
#include <polaris/EventLoopThreadPool.h>
#include <polaris/Sockets.h>
#include <polaris/TCPServer.h>

#include <cstdio>

using namespace Lute;

TCPServer::TCPServer(EventLoop* loop, const InetAddress& listenAddr,
                     const std::string& name, Option option)
    : loop_(loop),
      ipPort_(listenAddr.toIpPort()),
      name_(name),
      acceptor_(new Acceptor(loop, listenAddr, option == Option::kReusePort)),
      threadPool_(new EventLoopThreadPool(loop, name_)),
      connectionCallback_(defaultConnectionCallback),
      messageCallback_(defaultMessageCallback),
      nextConnId_(1) {
    acceptor_->setNewConnectionCallback(std::bind(&TCPServer::newConnection,
                                                  this, std::placeholders::_1,
                                                  std::placeholders::_2));
}

TCPServer::~TCPServer() {
    loop_->assertInLoopThread();
    LOG_TRACE << "TcpServer::~TcpServer [" << name_ << "] destructing";

    for (auto& item : connections_) {
        TCPConnectionPtr conn(item.second);
        item.second.reset();
        conn->getLoop()->runInLoop(
            std::bind(&TCPConnection::connectDestroyed, conn));
    }
}

void TCPServer::setThreadNum(int numThreads) {
    assert(0 <= numThreads);
    threadPool_->setThreadNum(numThreads);
}

void TCPServer::start() {
    if (started_.getAndSet(1) == 0) {
        threadPool_->start(threadInitCallback_);

        assert(!acceptor_->listenning());
        loop_->runInLoop(std::bind(&Acceptor::listen, get_pointer(acceptor_)));
    }
}

void TCPServer::newConnection(int sockfd, const InetAddress& peerAddr) {
    loop_->assertInLoopThread();
    EventLoop* ioLoop = threadPool_->getNextLoop();

    char buf[64];
    snprintf(buf, sizeof buf, "-%s#%d", ipPort_.c_str(), nextConnId_);
    ++nextConnId_;
    std::string connName = name_ + buf;

    LOG_INFO << "TcpServer::newConnection [" << name_ << "] - new connection ["
             << connName << "] from " << peerAddr.toIpPort();
    InetAddress localAddr(sockets::getLocalAddr(sockfd));

    // FIXME poll with zero timeout to double confirm the new connection
    // FIXME use make_shared if necessary
    TCPConnectionPtr conn(
        new TCPConnection(ioLoop, connName, sockfd, localAddr, peerAddr));
    connections_[connName] = conn;
    conn->setConnectionCallback(connectionCallback_);
    conn->setMessageCallback(messageCallback_);
    conn->setWriteCompleteCallback(writeCompleteCallback_);
    conn->setCloseCallback(std::bind(&TCPServer::removeConnection, this,
                                     std::placeholders::_1));  // FIXME: unsafe
    ioLoop->runInLoop(std::bind(&TCPConnection::connectEstablished, conn));
}

void TCPServer::removeConnection(const TCPConnectionPtr& conn) {
    // FIXME: unsafe
    loop_->runInLoop(std::bind(&TCPServer::removeConnectionInLoop, this, conn));
}

void TCPServer::removeConnectionInLoop(const TCPConnectionPtr& conn) {
    loop_->assertInLoopThread();
    LOG_INFO << "TcpServer::removeConnectionInLoop [" << name_
             << "] - connection " << conn->name();
    size_t n = connections_.erase(conn->name());
    (void)n;
    assert(n == 1);
    EventLoop* ioLoop = conn->getLoop();
    ioLoop->queueInLoop(std::bind(&TCPConnection::connectDestroyed, conn));
}
