/**
 * @file EventLoopThread.cc
 * @brief
 */

#include <polaris/EventLoop.h>
#include <polaris/EventLoopThread.h>

using namespace Lute;

EventLoopThread::EventLoopThread(const ThreadInitCallback& cb,
                                 const std::string& name)
    : loop_(nullptr),
      exiting_(false),
      thread_(std::bind(&EventLoopThread::threadFunc, this), name),
      mutex_(),
      cond_(mutex_),
      callback_(cb) {}

EventLoopThread::~EventLoopThread() {
    exiting_ = true;

    // Not 100% race-free, e.g. threadFunc could be running callback_
    if (loop_ != nullptr) {
        // Still a tiny chance to call destructed object, if threadFunc exits
        // just now. but when EventLoopThread destructs, usually programming is
        // exiting anyway.
        loop_->quit();
        thread_.join();
    }
}

EventLoop* EventLoopThread::startLoop() {
    assert(!thread_.started());
    thread_.start();

    EventLoop* loop = nullptr;
    {
        MutexLockGuard lock(mutex_);
        while (loop_ == nullptr) {
            cond_.wait();
        }

        loop = loop_;
    }

    return loop;
}

void EventLoopThread::threadFunc() {
    EventLoop loop;

    if (callback_) callback_(&loop);

    {
        MutexLockGuard lock(mutex_);

        loop_ = &loop;
        cond_.notify();
    }

    loop.loop();
    // assert(exiting_);
    MutexLockGuard lock(mutex_);
    loop_ = nullptr;
}
