/**
 * @file Acceptor.cc
 * @brief
 */

#include <LuteBase.h>
#include <fcntl.h>
#include <polaris/Acceptor.h>
#include <polaris/EventLoop.h>
#include <polaris/InetAddress.h>
#include <polaris/Sockets.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <functional>

using namespace Lute;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr,
                   bool reusePort)
    : loop_(loop),
      acceptSocket_(sockets::createNonblockingOrDie(listenAddr.family())),
      acceptChannel_(loop, acceptSocket_.fd()),
      listenning_(false),
      idleFd_(::open("/dev/null", O_RDONLY | O_CLOEXEC)) {
    assert(idleFd_ >= 0);
    acceptSocket_.setReuseAddr(true);
    acceptSocket_.setReusePort(reusePort);
    acceptSocket_.bindAddress(listenAddr);

    acceptChannel_.setReadCallback(std::bind(&Acceptor::handleRead, this));
}

Acceptor::~Acceptor() {
    acceptChannel_.disableAll();
    acceptChannel_.remove();

    ::close(idleFd_);
}

void Acceptor::listen() {
    loop_->assertInLoopThread();
    listenning_ = true;
    acceptSocket_.listen();
    acceptChannel_.enableReading();
}

void Acceptor::handleRead() {
    loop_->assertInLoopThread();
    InetAddress peerAddr;

    // FIXME loop until no more
    int connfd = acceptSocket_.accept(&peerAddr);

    // accept successful
    if (connfd >= 0) {
        std::string hostport = peerAddr.toIpPort();
        LOG_TRACE << "Accepts of " << hostport;
        if (newConnectionCallback_) {
            newConnectionCallback_(connfd, peerAddr);
        } else {
            sockets::close(connfd);
        }
    } else {  // accept failed
        LOG_SYSERR << "in Acceptor::handleRead";
        // Read the section named "The special problem of
        // accept()ing when you can't" in libev's doc.
        // By Marc Lehmann, author of libev.
        if (errno == EMFILE) {
            /* The per-process limit on the number of open file descriptors has
             * been reached. */
            ::close(idleFd_);
            idleFd_ = ::accept(acceptSocket_.fd(), nullptr, nullptr);
            ::close(idleFd_);
            idleFd_ = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
        }
    }
}
