/**
 * @file TimerQueue.h
 * @brief TimerQueue用timerfd实现定时,这有别于传统的设置 poll/epoll_wait
 * 的等待时长的办法 TimerQueue用std::map来管理Timer, 常用操作的复杂度是O(logN),
 * N为定时器数目
 */

#pragma once

#include <LuteBase.h>
#include <polaris/Callbacks.h>
#include <polaris/Channel.h>

#include <set>
#include <vector>
namespace Lute {

/// @brief Forward Declare
class EventLoop;
class Timer;
class TimerId;

///
/// A best efforts timer queue.
/// No guarantee that the callback will be on time.
///
class TimerQueue {
public:
    // non-copyable
    TimerQueue(const TimerQueue&) = delete;
    TimerQueue& operator=(TimerQueue&) = delete;

    explicit TimerQueue(EventLoop* loop);
    ~TimerQueue();

    ///
    /// Schedules the callback to be run at given time,
    /// repeats if @c interval > 0.0.
    ///
    /// Must be thread safe. Usually be called from other threads.
    TimerId addTimer(TimerCallback cb, Timestamp when, double interval);

    void cancel(TimerId timerId);

private:
    // FIXME: use unique_ptr<Timer> instead of raw pointers.
    // This requires heterogeneous comparison lookup (N3465) from C++14
    // so that we can find an T* in a set<unique_ptr<T>>.
    using Entry = std::pair<Timestamp, Timer*>;
    using TimerList = std::set<Entry>;
    using ActiveTimer = std::pair<Timer*, int64_t>;
    using ActiveTimerSet = std::set<ActiveTimer>;

    EventLoop* loop_;
    const int timerfd_;
    Channel timerfdChannel_;
    // Timer list sorted by expiration
    TimerList timers_;

    // for cancel()
    ActiveTimerSet activeTimers_;
    bool callingExpiredTimers_; /* atomic */
    ActiveTimerSet cancelingTimers_;

private:
    void addTimerInLoop(Timer* timer);
    void cancelInLoop(TimerId timerId);
    // called when timerfd alarms
    void handleRead();
    // move out all expired timers
    std::vector<Entry> getExpired(Timestamp now);
    void reset(const std::vector<Entry>& expired, Timestamp now);

    bool insert(Timer* timer);
};

}  // namespace Lute