/**
 * @file EventLoopThreadPool.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/EventLoop.h>
#include <polaris/EventLoopThread.h>
#include <polaris/EventLoopThreadPool.h>

#include <vector>

using namespace Lute;

EventLoopThreadPool::EventLoopThreadPool(EventLoop* baseLoop,
                                         const std::string& nameArg)
    : baseLoop_(baseLoop),
      name_(nameArg),
      started_(false),
      numThreads_(0),
      next_(0) {
    LOG_DEBUG << "thrs: " << numThreads_;
}

// Don't delete loop, it's stack variable
EventLoopThreadPool::~EventLoopThreadPool() {}

// 创建线程池 - one loop per thread
void EventLoopThreadPool::start(const ThreadInitCallback& cb) {
    assert(!started_);
    baseLoop_->assertInLoopThread();

    started_ = true;

    for (int i = 0; i < numThreads_; ++i) {
        char buf[name_.size() + 32];
        ::snprintf(buf, sizeof(buf), "%s %d", name_.c_str(), i);
        EventLoopThread* t = new EventLoopThread(cb, buf);
        threads_.push_back(std::unique_ptr<EventLoopThread>(t));
        loops_.push_back(t->startLoop());
    }

    if (numThreads_ == 0 && cb != nullptr) cb(baseLoop_);
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    baseLoop_->assertInLoopThread();
    assert(started_);
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        // FIXME round-robin
        loop = loops_[static_cast<std::vector<EventLoop*>::size_type>(next_)];
        ++next_;
        if (static_cast<size_t>(next_) >= loops_.size()) next_ = 0;
    }
    return loop;
}

EventLoop* EventLoopThreadPool::getLoopForHash(size_t hashCode) {
    baseLoop_->assertInLoopThread();
    EventLoop* loop = baseLoop_;

    if (!loops_.empty()) {
        loop = loops_[hashCode % loops_.size()];
    }
    return loop;
}

std::vector<EventLoop*> EventLoopThreadPool::getAllLoops() {
    baseLoop_->assertInLoopThread();
    assert(started_);

    return loops_.empty() ? std::vector<EventLoop*>(1, baseLoop_) : loops_;
}
