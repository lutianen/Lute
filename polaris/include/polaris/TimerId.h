/**
 * @file TimerId.h
 * @brief
 */

#pragma once

#include <cstdint>

namespace Lute {

/// Forward declare
class Timer;

/// @brief 一个不透明的标识符，用于取消 Timer
class TimerId {
public:
    TimerId() : timer_(nullptr), sequence_(0) {}

    TimerId(Timer* timer, int64_t seq) : timer_(timer), sequence_(seq) {}

    // default copy-ctor, dtor and assignment are okay

    friend class TimerQueue;

private:
    Timer* timer_;
    int64_t sequence_;
};

}  // namespace Lute
