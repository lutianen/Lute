/**
 * @file Channel.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/Channel.h>
#include <polaris/EventLoop.h>
#include <poll.h>

#include <sstream>

using namespace Lute;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = POLLIN | POLLPRI;  // POLLPRI 用于带外数据
const int Channel::kWriteEvent = POLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : loop_(loop),
      fd_(fd),
      events_(0),
      revents_(0),
      index_(-1),
      logHup_(true),
      tied_(false),
      eventHandling_(false),
      addedToLoop_(false) {}

Channel::~Channel() {
    assert(!eventHandling_);
    assert(!addedToLoop_);
    if (loop_->isInLoopThread()) {
        assert(!loop_->hasChannel(this));
    }
}

void Channel::handleEvent(Timestamp receiveTime) {
    std::shared_ptr<void> guard;
    if (tied_) {
        // 当 tie_ 指向的对象被销毁时，guard 为 nullptr
        guard = tie_.lock();
        if (guard) handleEventWithGuard(receiveTime);
    } else {
        handleEventWithGuard(receiveTime);
    }
}

void Channel::tie(const std::shared_ptr<void>& obj) {
    tie_ = obj;
    tied_ = true;
}

/**
 * @brief Update Channel
 */
void Channel::update() {
    addedToLoop_ = true;
    loop_->updateChannel(this);
}

void Channel::handleEventWithGuard(Timestamp receiveTime) {
    eventHandling_ = true;
    LOG_TRACE << reventsToString();

    // POLLHUP - Hang up
    // 与文件描述符相关的连接已经被挂起或者被挂断，
    // 例如在一个 TCP连接中，当对端关闭连接时，
    // 本地端将会收到一个 FIN 报文并且连接被挂起
    if ((revents_ & POLLHUP) && !(revents_ & POLLIN)) {
        if (logHup_) {
            LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLHUP";
        }
        if (closeCallback_) closeCallback_();
    }

    // POLLNVAL - Invalid fd
    if (revents_ & POLLNVAL) {
        LOG_WARN << "fd = " << fd_ << " Channel::handle_event() POLLNVAL";
    }

    // POLLERR - Error
    if (revents_ & (POLLERR | POLLNVAL)) {
        if (errorCallback_) errorCallback_();
    }
    // POLLIN / POLLPRI / POLLRDHUP - Read
    if (revents_ & (POLLIN | POLLPRI | POLLRDHUP)) {
        if (readCallback_) readCallback_(receiveTime);
    }
    // POLLOUT - Write
    if (revents_ & POLLOUT) {
        if (writeCallback_) writeCallback_();
    }
    eventHandling_ = false;
}

/**
 * @brief Remove Channel
 */
void Channel::remove() {
    assert(isNoneEvent());
    addedToLoop_ = false;
    loop_->removeChannel(this);
}

std::string Channel::reventsToString() const {
    return eventsToString(fd_, revents_);
}

std::string Channel::eventsToString() const {
    return eventsToString(fd_, events_);
}

/**
 * @brief Event format:
 *  POLLIN / POLLPRI / POLLOUT / POLLHUP / POLLRDHUP / POLLERR / POLLNVAL
 */
std::string Channel::eventsToString(int fd, int ev) {
    std::ostringstream oss;
    oss << fd << ": ";
    if (ev & POLLIN) oss << "IN ";
    if (ev & POLLPRI) oss << "PRI ";
    if (ev & POLLOUT) oss << "OUT ";
    if (ev & POLLHUP) oss << "HUP ";
    if (ev & POLLRDHUP) oss << "RDHUP ";
    if (ev & POLLERR) oss << "ERR ";
    if (ev & POLLNVAL) oss << "NVAL ";

    return oss.str();
}
