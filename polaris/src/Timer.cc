/**
 * @file Timer.cc
 * @brief
 */

#include <polaris/Timer.h>

using namespace Lute;

AtomicInt64 Timer::s_numCreated_;

void Timer::restart(Timestamp now) {
    if (repeat_)
        expiration_ = addTime(now, interval_);
    else
        expiration_ = Timestamp::invalid();
}
