/**
 * @file EventLoopThread.h
 * @brief
 */

#pragma once

#include <LuteBase.h>

namespace Lute {

class EventLoop;

class EventLoopThread {
public:
    using ThreadInitCallback = std::function<void(EventLoop* loop)>;

    // noncopyable
    EventLoopThread(const EventLoopThread&) = delete;
    EventLoopThread& operator=(EventLoopThread&) = delete;

    EventLoopThread(const ThreadInitCallback& cb = ThreadInitCallback(),
                    const std::string& name = std::string());
    ~EventLoopThread();

    EventLoop* startLoop();

private:
    EventLoop* loop_ GUARDED_BY(mutex_);
    bool exiting_;
    Thread thread_;

    MutexLock mutex_;
    Condition cond_ GUARDED_BY(mutex_);

    ThreadInitCallback callback_;

    void threadFunc();
};

}  // namespace Lute
