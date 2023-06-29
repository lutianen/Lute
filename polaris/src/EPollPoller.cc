/**
 * @file EPollPoller.cc
 * @brief
 */

#include <LuteBase.h>
#include <polaris/Channel.h>
#include <polaris/EPollPoller.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>

#include <cassert>
#include <cerrno>
#include <vector>

using namespace Lute;

// On Linux, the constants of poll(2) and epoll(4) are expected to be the same.
static_assert(EPOLLIN == POLLIN, "epoll uses same flag values as poll");
static_assert(EPOLLPRI == POLLPRI, "epoll uses same flag values as poll");
static_assert(EPOLLOUT == POLLOUT, "epoll uses same flag values as poll");
static_assert(EPOLLRDHUP == POLLRDHUP, "epoll uses same flag values as poll");
static_assert(EPOLLERR == POLLERR, "epoll uses same flag values as poll");
static_assert(EPOLLHUP == POLLHUP, "epoll uses same flag values as poll");

namespace {
const int kNew = -1;
const int kAdded = 1;
const int kDeleted = 2;
}  // namespace

/**
 * @brief Set eventLoop and Create epoll
 */
EPollPoller::EPollPoller(EventLoop* loop)
    : Poller(loop),
      epollfd_(::epoll_create1(EPOLL_CLOEXEC)),
      events_(kInitEventListSize) {
    if (epollfd_ < 0) {
        LOG_SYSFATAL << "EPollPoller::EPollPoller";
    }
}

/**
 * @brief Close epollfd
 */
EPollPoller::~EPollPoller() { ::close(epollfd_); }

/**
 * @brief Call `epoll_wait`
 *
 * @param timeoutMs 最大等待时间，设置为-1表示一直等待
 * @param activeChannels
 * @return Timestamp
 */
Timestamp EPollPoller::poll(int timeoutMs, ChannelList* activeChannels) {
    LOG_TRACE << "fd total count " << channels_.size();
    int numEvents = ::epoll_wait(epollfd_, &*events_.begin(),
                                 static_cast<int>(events_.size()), timeoutMs);
    int savedErrno = errno;
    Timestamp now(Timestamp::now());

    /* Readable events */
    if (numEvents > 0) {
        LOG_TRACE << numEvents << " events happened";
        fillActiveChannels(numEvents, activeChannels);
        if (static_cast<size_t>(numEvents) == events_.size()) {
            events_.resize(events_.size() * 2);
        }
    } else if (numEvents == 0) {
        /* timeout */
        LOG_TRACE << "nothing happened";
    } else {
        // error happens, log uncommon ones
        if (savedErrno != EINTR) {
            errno = savedErrno;
            LOG_SYSERR << "EPollPoller::poll()";
        }
    }
    return now;
}

void EPollPoller::fillActiveChannels(int numEvents,
                                     ChannelList* activeChannels) const {
    assert(static_cast<size_t>(numEvents) <= events_.size());

    // 遍历 events_，将其中的 Channel* 指针取出，放入 activeChannels 中
    for (int i = 0; i < numEvents; ++i) {
        Channel* channel = static_cast<Channel*>(
            events_[static_cast<std::vector<EventList>::size_type>(i)]
                .data.ptr);

        int fd = channel->fd();
        auto it = channels_.find(fd);
        assert(it != channels_.end());
        assert(it->second == channel);

        channel->set_revents(static_cast<int>(
            events_[static_cast<std::vector<EventList>::size_type>(i)].events));
        activeChannels->push_back(channel);
    }
}

void EPollPoller::updateChannel(Channel* channel) {
    Poller::assertInLoopThread();
    const int index = channel->index();
    LOG_TRACE << "fd = " << channel->fd() << " events = " << channel->events()
              << " index = " << index;
    if (index == kNew || index == kDeleted) {
        // a new one, add with EPOLL_CTL_ADD
        int fd = channel->fd();
        if (index == kNew) {
            assert(channels_.find(fd) == channels_.end());
            channels_[fd] = channel;
        } else  // index == kDeleted
        {
            assert(channels_.find(fd) != channels_.end());
            assert(channels_[fd] == channel);
        }

        channel->set_index(kAdded);
        update(EPOLL_CTL_ADD, channel);
    } else {
        // update existing one with EPOLL_CTL_MOD/DEL
        int fd = channel->fd();
        (void)fd;
        assert(channels_.find(fd) != channels_.end());
        assert(channels_[fd] == channel);
        assert(index == kAdded);
        if (channel->isNoneEvent()) {
            update(EPOLL_CTL_DEL, channel);
            channel->set_index(kDeleted);
        } else {
            update(EPOLL_CTL_MOD, channel);
        }
    }
}

void EPollPoller::removeChannel(Channel* channel) {
    Poller::assertInLoopThread();
    int fd = channel->fd();
    LOG_TRACE << "fd = " << fd;
    assert(channels_.find(fd) != channels_.end());
    assert(channels_[fd] == channel);
    assert(channel->isNoneEvent());
    int index = channel->index();
    assert(index == kAdded || index == kDeleted);
    size_t n = channels_.erase(fd);
    (void)n;
    assert(n == 1);

    if (index == kAdded) {
        update(EPOLL_CTL_DEL, channel);
    }
    channel->set_index(kNew);
}

void EPollPoller::update(int operation, Channel* channel) {
    struct epoll_event event;
    memZero(&event, sizeof(event));
    event.events = static_cast<uint32_t>(channel->events());
    event.data.ptr = channel;
    int fd = channel->fd();
    LOG_TRACE << "epoll_ctl op = " << operationToString(operation)
              << " fd = " << fd << " event = { " << channel->eventsToString()
              << " }";

    // NOTE ::epoll_ctl
    if (::epoll_ctl(epollfd_, operation, fd, &event) < 0) {
        if (operation == EPOLL_CTL_DEL) {
            LOG_SYSERR << "epoll_ctl op =" << operationToString(operation)
                       << " fd =" << fd;
        } else {
            LOG_SYSFATAL << "epoll_ctl op =" << operationToString(operation)
                         << " fd =" << fd;
        }
    }
}

/// @brief Get the operation
/// @return const char *
const char* EPollPoller::operationToString(int op) {
    switch (op) {
        case EPOLL_CTL_ADD:
            return "ADD";
        case EPOLL_CTL_DEL:
            return "DEL";
        case EPOLL_CTL_MOD:
            return "MOD";
        default:
            assert(false && "ERROR op");
            return "Unknown Operation";
    }
}
