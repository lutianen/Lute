/**
 * @file Channel.h
 * @brief
 */

#pragma once

#include <LuteBase.h>

#include <functional>
#include <memory>

namespace Lute {

// 前向声明
class EventLoop;

/**
 * @brief A selectable I/O Channel
 *  This class `Channel` doesn't own the file descriptor.
 *  The file descriptor could be a socket, an eventfd, a timerfd or
 * signalfd. 只负责一个文件描述符，对不同的服务、不同的事件类型，
 * 都可以在类中进行不同的处理，而不是仅仅拿到一个int类型的文件描述符
 */
class Channel {
public:
    using EventCallback = std::function<void()>;
    using ReadEventCallback = std::function<void(Timestamp)>;

    // noncopyable
    Channel(const Channel&) = delete;
    Channel& operator=(Channel&) = delete;

    Channel(EventLoop* loop, int fd);
    ~Channel();

    void handleEvent(Timestamp receiveTime);

    /// @brief Set callback
    inline void setReadCallback(ReadEventCallback cb) {
        readCallback_ = std::move(cb);
    }
    inline void setWriteCallback(EventCallback cb) {
        writeCallback_ = std::move(cb);
    }
    inline void setCloseCallback(EventCallback cb) {
        closeCallback_ = std::move(cb);
    }
    inline void setErrorCallback(EventCallback cb) {
        errorCallback_ = std::move(cb);
    }

    /// Tie this channel to the owner object managed by shared_ptr,
    /// prevent the owner object being destroyed in handleEvent.
    void tie(const std::shared_ptr<void>&);

    inline int fd() const { return fd_; }
    inline int events() const { return events_; }
    inline void set_revents(int revt) { revents_ = revt; }
    // used by pollers
    // int revents() const { return revents_; }
    inline bool isNoneEvent() const { return events_ == kNoneEvent; }

    inline void enableReading() {
        events_ |= kReadEvent;
        update();
    }
    inline void disableReading() {
        events_ &= ~kReadEvent;
        update();
    }
    inline void enableWriting() {
        events_ |= kWriteEvent;
        update();
    }
    inline void disableWriting() {
        events_ &= ~kWriteEvent;
        update();
    }
    inline void disableAll() {
        events_ = kNoneEvent;
        update();
    }
    inline bool isWriting() const { return events_ & kWriteEvent; }
    inline bool isReading() const { return events_ & kReadEvent; }

    // for Poller
    inline int index() { return index_; }
    inline void set_index(int idx) { index_ = idx; }

    // for debug
    std::string reventsToString() const;
    std::string eventsToString() const;

    inline void doNotLogHup() { logHup_ = false; }

    inline EventLoop* ownerLoop() { return loop_; }
    void remove();

private:
    static const int kNoneEvent;
    static const int kReadEvent;
    static const int kWriteEvent;

    EventLoop* loop_;
    /// fileDescriptor
    const int fd_;
    /// 希望监听这个文件描述符的哪些事件，因为不同事件的处理方式不一样
    int events_;
    /// It is received event types of epoll or poll
    /// 文件描述符正在发生的事件
    int revents_;
    int index_;
    bool logHup_;

    // TODO tie_ ???
    std::weak_ptr<void> tie_;
    bool tied_;
    bool eventHandling_;
    bool addedToLoop_;
    ReadEventCallback readCallback_;
    EventCallback writeCallback_;
    EventCallback closeCallback_;
    EventCallback errorCallback_;

    static std::string eventsToString(int fd, int ev);

    void update();
    void handleEventWithGuard(Timestamp receiveTime);
};

}  // namespace Lute
