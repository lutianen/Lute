/**
 * @file Poller.h
 * @brief PollPoller 和 EPollPoller 的基类，
 *  采用“电平触发(LT, Level Trigger)”语意 LT模式，
 *  是 epoll 的默认工作模式，这种情况下 epoll 相当于一个效率较高的 poll
 *      - 读事件触发后，可以按需收取想要的字节数，
 *        不用把本次接收到的数据收取干净（即不用循环到recv或者read函数返回-1，
 *        错误码为 EWOULDBLOCK 或 EAGAIN）
 *      - 不需要写事件一定要及时移除，避免不必要的触发，浪费 CPU 资源
 *      - 可以自由决定每次收取多少字节
 *       （对于普通socket）或何时接收连接（对于侦听socket），但是可能会导致多次触发
 *
 *  ET(Edge Trigger) 模式，当往 epoll 内核事件表中注册一个文件描述符上的 EPOLLET
 * 事件时，epoll 将以 ET 模式来操作该文件描述符，是 epoll 的高效工作模式 ET
 * 使用准则: 只有出现EAGAIN错误才调用epoll_wait
 *      - 读事件必须把数据收取干净，因为你不一定有下一次机会再收取数据了，
 *        即使有机会，也可能存在上次没读完的数据没有及时处理，造成客户端响应延迟
 *      - 写事件触发后，如果还需要下一次的写事件触发来驱动任务
 *       （例如发上次剩余的数据），你需要继续注册一次检测可写事件
 *      - 必须每次都要将数据收完（对于普通 socket）
 *        或必须理解调用 accept 接收连接（对于侦听socket），其优点是触发次数少
 *
 * ET 模式在很大程度上降低了一个 epoll 事件被重复触发的次数，因此效率要比 LT
 * 模式高
 *
 */

#pragma once

#include <LuteBase.h>
#include <polaris/EventLoop.h>

#include <map>
#include <vector>

namespace Lute {

class Channel;

///
/// Base class for IO Multiplexing
///
/// This class doesn't own the Channel objects.
class Poller {
public:
    using ChannelList = std::vector<Channel*>;

    // noncopyable
    Poller(const Poller&) = delete;
    Poller& operator=(Poller&) = delete;

    Poller(EventLoop* loop);
    virtual ~Poller();

    /// Polls the I/O events.
    /// Must be called in the loop thread.
    virtual Timestamp poll(int timeoutMs, ChannelList* activeChannels) = 0;

    /// Changes the interested I/O events.
    /// Must be called in the loop thread.
    virtual void updateChannel(Channel* channel) = 0;

    /// Remove the channel, when it destructs.
    /// Must be called in the loop thread.
    virtual void removeChannel(Channel* channel) = 0;

    virtual bool hasChannel(Channel* channel) const;

    static Poller* newDefaultPoller(EventLoop* loop);

    void assertInLoopThread() const { ownerLoop_->assertInLoopThread(); }

protected:
    // <fd, Channel>
    using ChannelMap = std::map<int, Channel*>;
    ChannelMap channels_;

private:
    EventLoop* ownerLoop_;
};

}  // namespace Lute
