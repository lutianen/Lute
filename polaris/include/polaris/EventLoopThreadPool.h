/**
 * @file EventLoopThreadPool.h
 * @brief
 */

#pragma once

#include <LuteBase.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

namespace Lute {

class EventLoop;
class EventLoopThread;

class EventLoopThreadPool {
public:
    using ThreadInitCallback = std::function<void(EventLoop*)>;

    // noncopyable
    EventLoopThreadPool(const EventLoopThreadPool&) = delete;
    EventLoopThreadPool& operator=(EventLoopThreadPool&) = delete;

    EventLoopThreadPool(EventLoop* baseLoop, const std::string& nameArg);
    ~EventLoopThreadPool();

    inline void setThreadNum(int numThreads) {
        LOG_DEBUG << "numThreads: " << numThreads_;
        numThreads_ = numThreads;
    }
    void start(const ThreadInitCallback& cb = ThreadInitCallback());

    /// @brief valid after calling start()
    /// round-robin
    /// @return EventLoop*
    EventLoop* getNextLoop();

    /// @brief with the same hash code, it will always return the same
    /// EventLoop
    /// @param hashCOde
    /// @return EventLoop*
    EventLoop* getLoopForHash(size_t hashCode);

    std::vector<EventLoop*> getAllLoops();

    inline bool started() const { return started_; }

    inline const std::string& name() const { return name_; }

private:
    // baseLoop_ is the main loop
    EventLoop* baseLoop_;
    std::string name_;
    bool started_;
    // 线程池中线程的数量
    int numThreads_;
    int next_;

    // 线程池
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    // EventLoop pool
    std::vector<EventLoop*> loops_;
};

}  // namespace Lute
