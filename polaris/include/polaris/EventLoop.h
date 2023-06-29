/**
 * @file EventLoop.h
 * @brief
 */

#pragma once

#include <LuteBase.h>
#include <polaris/Callbacks.h>
#include <polaris/TimerId.h>

#include <atomic>
#include <functional>
#include <vector>
namespace Lute {

class Channel;
class Poller;
class TimerQueue;

///
/// Reactor, at most one per thread.
///
/// This is an interface class, so don't expose too much details.
class EventLoop {
    EventLoop(const EventLoop&) = delete;
    EventLoop& operator=(EventLoop&) = delete;

public:
    using Functor = std::function<void()>;

private:
    using ChannelList = std::vector<Channel*>;

    /* atomic */
    bool looping_;
    std::atomic<bool> quit_;
    /* atomic */
    bool eventHandling_;
    /* atomic */
    bool callingPendingFunctors_;
    int64_t iteration_;
    const pid_t threadId_;
    Timestamp pollReturnTime_;

    // poller
    std::unique_ptr<Poller> poller_;
    std::unique_ptr<TimerQueue> timerQueue_;
    int wakeupFd_;
    // unlike in TimerQueue, which is an internal class,
    // we don't expose Channel to client.
    std::unique_ptr<Channel> wakeupChannel_;
    Lute::any context_;

    // scratch variables
    ChannelList activeChannels_;
    Channel* currentActiveChannel_;

    mutable MutexLock mutex_;
    std::vector<Functor> pendingFunctors_ GUARDED_BY(mutex_);

private:
    void abortNotInLoopThread();
    // waked up
    void handleRead();
    void doPendingFunctors();

    // DEBUG
    void printActiveChannels() const;

public:
    EventLoop();
    // force out-line dtor, for std::unique_ptr members.
    ~EventLoop();
    /// Loops forever.
    /// Must be called in the same thread as creation of the object.
    void loop();

    /// Quits loop.
    /// This is not 100% thread safe, if you call through a raw pointer,
    /// better to call through shared_ptr<EventLoop> for 100% safety.
    void quit();

    /// Time when poll returns, usually means data arrival.
    Timestamp pollReturnTime() const { return pollReturnTime_; }

    int64_t iteration() const { return iteration_; }

    /// Runs callback immediately in the loop thread.
    /// It wakes up the loop, and run the cb.
    /// If in the same loop thread, cb is run within the function.
    /// Safe to call from other threads.
    void runInLoop(Functor cb);
    /// Queues callback in the loop thread.
    /// Runs after finish pooling.
    /// Safe to call from other threads.
    void queueInLoop(Functor cb);

    size_t queueSize() const;

    // FIXME timers

    /// Runs callback at 'time'.
    /// Safe to call from other threads.
    TimerId runAt(Timestamp time, TimerCallback cb);
    /// Runs callback after @c delay seconds.
    /// Safe to call from other threads.
    TimerId runAfter(double delay, TimerCallback cb);
    /// Runs callback every @c interval seconds.
    /// Safe to call from other threads.
    TimerId runEvery(double interval, TimerCallback cb);
    /// Cancels the timer.
    /// Safe to call from other threads.
    void cancel(TimerId timerId);

    // internal usage
    void wakeup();

    // Channel
    void updateChannel(Channel* channel);
    void removeChannel(Channel* channel);
    bool hasChannel(Channel* channel);

    // pid_t threadId() const { return threadId_; }
    /**
     * @brief `threadId_ == CurrentThread::tid()`
     */
    inline void assertInLoopThread() {
        if (!isInLoopThread()) abortNotInLoopThread();
    }
    inline bool isInLoopThread() const {
        return threadId_ == CurrentThread::tid();
    }
    // bool callingPendingFunctors() const { return
    // callingPendingFunctors_; }
    inline bool eventHandling() const { return eventHandling_; }

    inline void setContext(const Lute::any& context) { context_ = context; }

    inline const Lute::any& getContext() const { return context_; }

    inline Lute::any* getMutableContext() { return &context_; }

    static EventLoop* getEventLoopOfCurrentThread();
};

}  // namespace Lute