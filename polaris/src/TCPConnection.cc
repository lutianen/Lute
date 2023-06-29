/**
 * @file TCPConnection.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/Callbacks.h>
#include <polaris/Channel.h>
#include <polaris/EventLoop.h>
#include <polaris/InetAddress.h>
#include <polaris/Sockets.h>
#include <polaris/TCPConnection.h>

using namespace Lute;

void Lute::defaultConnectionCallback(const TCPConnectionPtr& conn) {
    LOG_TRACE << conn->localAddress().toIpPort() << " -> "
              << conn->peerAddress().toIpPort() << " is "
              << (conn->connected() ? "UP" : "DOWN");
    // do not call conn->forceClose(), because some users want to register
    // message callback only.
}

void Lute::defaultMessageCallback(const TCPConnectionPtr& conn, Buffer* buffer,
                                  Timestamp receiveTime) {
    // FIXME: conn receiveTime is not used
    buffer->retrieveAll();
    (void)conn;
    (void)receiveTime;
}

// ----------

TCPConnection::TCPConnection(EventLoop* loop, const std::string& name,
                             int sockfd, const InetAddress& localAddr,
                             const InetAddress& perrAddr)
    : loop_(loop),
      name_(name),
      state_(StateE::kConnecting),
      reading_(false),
      socket_(new Socket(sockfd)),
      channel_(new Channel(loop, sockfd)),
      localAddr_(localAddr),
      peerAddr_(perrAddr),
      highWaterMark_(64 * 1024 * 1024) {
    channel_->setReadCallback(
        std::bind(&TCPConnection::handleRead, this, std::placeholders::_1));
    channel_->setWriteCallback(std::bind(&TCPConnection::handleWrite, this));
    channel_->setCloseCallback(std::bind(&TCPConnection::handleClose, this));
    channel_->setErrorCallback(std::bind(&TCPConnection::handleError, this));
    LOG_DEBUG << "TCPConnection::ctor[" << name_ << "] at " << this
              << " fd=" << sockfd;
    socket_->setKeepAlive(true);
}

TCPConnection::~TCPConnection() {
    LOG_DEBUG << "TCPConnection::dtor[" << name_ << "] at " << this
              << " fd=" << channel_->fd() << " state=" << stateToString();
    assert(state_ == StateE::kDisconnected);
}

bool TCPConnection::getTcpInfo(struct tcp_info* tcpi) const {
    return this->socket_->getTcpInfo(tcpi);
}

std::string TCPConnection::getTcpInfoString() const {
    char buf[1024];
    buf[0] = '\0';
    socket_->getTcpInfoString(buf, sizeof(buf));
    return buf;
}

/**
 * @brief send - NonBlocking, thread safe, atomic.
 *
 * @param message
 * @param len
 * @return void: User don't care about the number of sent bytes.
 */
void TCPConnection::send(const void* message, int len) {
    // send(StringPiece(static_cast<const char*>(message),
    //                  static_cast<std::basic_string<char>::size_type>(len)));

    /// XXX std::sting_view
    send(std::string(static_cast<const char*>(message),
                     static_cast<std::basic_string<char>::size_type>(len)));
}

/**
 * @brief send - NonBlocking, thread safe, atomic.
 *
 * @param message
 * @return void: User don't care about the number of sent bytes.
 */
// void TCPConnection::send(const StringPiece& message) {
/// XXX std::string_view
void TCPConnection::send(const std::string& message) {
    if (state_ == StateE::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(message);
        } else {
            // XXX string_view
            void (TCPConnection::*fp)(const std::string& message) =
                &TCPConnection::sendInLoop;
#if __cplusplus >= 201703L
            loop_->runInLoop(std::bind(fp, this, std::string(message)));
#else
            loop_->runInLoop(std::bind(fp,
                                       this,  // FIXME
                                       message));
// std::forward<string>(message)));
#endif
        }
    }
}

// FIXME efficiency!!!
/**
 * @brief send - NonBlocking, thread safe, atomic.
 *
 * @param buffer
 * @return void: User don't care about the number of sent bytes.
 */
void TCPConnection::send(Buffer* buffer) {
    if (state_ == StateE::kConnected) {
        if (loop_->isInLoopThread()) {
            sendInLoop(buffer->peek(), buffer->readableBytes());
            buffer->retrieveAll();
        } else {
            // XXX string_view
            void (TCPConnection::*fp)(const std::string& message) =
                &TCPConnection::sendInLoop;
            loop_->runInLoop(std::bind(fp,
                                       this,  // FIXME
                                       buffer->retrieveAllAsString()));
            // std::forward<string>(message)));
        }
    }
}

/// XXX string_view
void TCPConnection::sendInLoop(const std::string& message) {
    sendInLoop(message.data(), message.size());
}

void TCPConnection::sendInLoop(const void* message, size_t len) {
    loop_->assertInLoopThread();
    ssize_t nwrote = 0;
    size_t remaining = len;
    bool faultError = false;

    if (state_ == StateE::kDisconnected) {
        LOG_WARN << "disconnected, give up writing";
        return;
    }

    // if nothing in output queue, try writing directly
    if (!channel_->isWriting() && outputBuffer_.readableBytes() == 0) {
        nwrote = sockets::write(channel_->fd(), message, len);
        LOG_TRACE << "write " << nwrote << " bytes to fd=" << channel_->fd();

        if (nwrote >= 0) {
            remaining = len - static_cast<unsigned long>(nwrote);
            // write complete -> Call cb
            if (remaining == 0 && writeCompleteCallback_)
                loop_->queueInLoop(
                    std::bind(writeCompleteCallback_, shared_from_this()));

        } else /* nwrote < 0 */ {
            nwrote = 0;
            if (errno != EWOULDBLOCK) {
                LOG_SYSERR << "TCPConnection::sendInLoop";
                if (errno == EPIPE ||
                    errno == ECONNRESET)  // FIXME: any others?
                {
                    faultError = true;
                }
            }
        }
    }

    assert(remaining <= len);
    if (!faultError && remaining > 0) {
        size_t oldLen = outputBuffer_.readableBytes();
        if (oldLen + remaining >= highWaterMark_ && oldLen < highWaterMark_ &&
            highWaterMarkCallback_) {
            loop_->queueInLoop(std::bind(highWaterMarkCallback_,
                                         shared_from_this(),
                                         oldLen + remaining));
        }
        outputBuffer_.append(reinterpret_cast<const char*>(message) + nwrote,
                             remaining);
        if (!channel_->isWriting()) {
            channel_->enableWriting();
        }
    }
}

void TCPConnection::shutdown() {
    // FIXME: use compare and swap
    if (state_ == StateE::kConnected) {
        setState(StateE::kDisconnecting);
        // FIXME: shared_from_this()?
        loop_->runInLoop(std::bind(&TCPConnection::shutdownInLoop, this));
    }
}

void TCPConnection::shutdownInLoop() {
    loop_->assertInLoopThread();
    if (!channel_->isWriting()) {
        // we are not writing
        socket_->shutdownWrite();
    }
}

// void TCPConnection::shutdownAndForceCloseAfter(double seconds)
// {
//   // FIXME: use compare and swap
//   if (state_ == kConnected)
//   {
//     setState(kDisconnecting);
//     loop_->runInLoop(std::bind(&TCPConnection::shutdownAndForceCloseInLoop,
//     this, seconds));
//   }
// }

// void TCPConnection::shutdownAndForceCloseInLoop(double seconds)
// {
//   loop_->assertInLoopThread();
//   if (!channel_->isWriting())
//   {
//     // we are not writing
//     socket_->shutdownWrite();
//   }
//   loop_->runAfter(
//       seconds,
//       makeWeakCallback(shared_from_this(),
//                        &TCPConnection::forceCloseInLoop));
// }

void TCPConnection::forceClose() {
    // FIXME: use compare and swap
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        setState(StateE::kDisconnecting);
        loop_->queueInLoop(
            std::bind(&TCPConnection::forceCloseInLoop, shared_from_this()));
    }
}

void TCPConnection::forceCloseWithDelay(double seconds) {
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        setState(StateE::kDisconnecting);
        loop_->runAfter(
            seconds,
            makeWeakCallback(
                shared_from_this(),
                &TCPConnection::forceClose));  // not forceCloseInLoop to avoid
                                               // race condition
    }
}

void TCPConnection::forceCloseInLoop() {
    loop_->assertInLoopThread();
    if (state_ == StateE::kConnected || state_ == StateE::kDisconnecting) {
        // as if we received 0 byte in handleRead();
        handleClose();
    }
}

const char* TCPConnection::stateToString() const {
    switch (state_) {
        case StateE::kDisconnected:
            return "kDisconnected";
        case StateE::kConnecting:
            return "kConnecting";
        case StateE::kConnected:
            return "kConnected";
        case StateE::kDisconnecting:
            return "kDisconnecting";
        default:
            return "unknown state";
    }
}

void TCPConnection::setTcpNoDelay(bool on) { socket_->setTcpNoDelay(on); }

void TCPConnection::startRead() {
    loop_->runInLoop(std::bind(&TCPConnection::startReadInLoop, this));
}
void TCPConnection::startReadInLoop() {
    loop_->assertInLoopThread();
    if (!reading_ || !channel_->isReading()) {
        channel_->enableReading();
        reading_ = true;
    }
}

void TCPConnection::stopRead() {
    loop_->runInLoop(std::bind(&TCPConnection::stopReadInLoop, this));
}
void TCPConnection::stopReadInLoop() {
    loop_->assertInLoopThread();
    if (reading_ || channel_->isReading()) {
        channel_->disableReading();
        reading_ = false;
    }
}

void TCPConnection::connectEstablished() {
    loop_->assertInLoopThread();
    assert(state_ == StateE::kConnecting);
    setState(StateE::kConnected);
    channel_->tie(shared_from_this());
    channel_->enableReading();

    connectionCallback_(shared_from_this());
}

void TCPConnection::connectDestroyed() {
    loop_->assertInLoopThread();
    if (state_ == StateE::kConnected) {
        setState(StateE::kDisconnected);
        channel_->disableAll();

        connectionCallback_(shared_from_this());
    }
    channel_->remove();
}

void TCPConnection::handleRead(Timestamp receiveTime) {
    loop_->assertInLoopThread();
    int savedErrno = 0;
    ssize_t n = inputBuffer_.readFd(channel_->fd(), &savedErrno);

    // 正常读到数据
    if (n > 0) {
        messageCallback_(shared_from_this(), &inputBuffer_, receiveTime);
    } else if (n == 0) /* 读取到文件末尾，则关闭 TCP 连接 */ {
        handleClose();
    } else /* 读取出错 */ {
        errno = savedErrno;
        LOG_SYSERR << "TCPConnection::handleRead";
        handleError();
    }
}

void TCPConnection::handleWrite() {
    loop_->assertInLoopThread();
    /* 可写状态 */
    if (channel_->isWriting()) {
        ssize_t n = sockets::write(channel_->fd(), outputBuffer_.peek(),
                                   outputBuffer_.readableBytes());

        /* 正常写入 n bytes*/
        if (n > 0) {
            LOG_TRACE << "write " << n
                      << " bytes data to fd = " << channel_->fd();
            outputBuffer_.retrieve(static_cast<size_t>(n));
            if (outputBuffer_.readableBytes() == 0) {
                channel_->disableWriting();
                if (writeCompleteCallback_) {
                    loop_->queueInLoop(
                        std::bind(writeCompleteCallback_, shared_from_this()));
                }
                if (state_ == StateE::kDisconnecting) {
                    shutdownInLoop();
                }
            }
        } else /* 写入出错 */ {
            LOG_SYSERR << "TCPConnection::handleWrite";
            // if (state_ == kDisconnecting)
            // {
            //   shutdownInLoop();
            // }
        }
    } else /* TCP 连接关闭 */ {
        LOG_TRACE << "Connection fd = " << channel_->fd()
                  << " is down, no more writing";
    }
}

void TCPConnection::handleClose() {
    loop_->assertInLoopThread();
    LOG_TRACE << "fd = " << channel_->fd() << " state = " << stateToString();
    assert(state_ == StateE::kConnected || state_ == StateE::kDisconnecting);
    // we don't close fd, leave it to dtor, so we can find leaks easily.
    setState(StateE::kDisconnected);
    channel_->disableAll();

    TCPConnectionPtr guardThis(shared_from_this());
    connectionCallback_(guardThis);
    // must be the last line
    closeCallback_(guardThis);
}

void TCPConnection::handleError() {
    int err = sockets::getSocketError(channel_->fd());
    LOG_ERROR << "TCPConnection::handleError [" << name_
              << "] - SO_ERROR = " << err << " " << strerror_tl(err);
}
