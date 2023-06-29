/**
 * @file Callbacks.h
 * @brief
 */

#pragma once

#include <LuteBase.h>

#include <functional>
#include <memory>

namespace Lute {
template <typename T>
inline T* get_pointer(const std::shared_ptr<T>& ptr) {
    return ptr.get();
}

template <typename T>
inline T* get_pointer(const std::unique_ptr<T>& ptr) {
    return ptr.get();
}

// All client visible callbacks go here.
class Buffer;
class TCPConnection;
using TCPConnectionPtr = std::shared_ptr<TCPConnection>;
using TimerCallback = std::function<void()>;
using ConnectionCallback = std::function<void(const TCPConnectionPtr&)>;
using CloseCallback = std::function<void(const TCPConnectionPtr&)>;
using WriteCompleteCallback = std::function<void(const TCPConnectionPtr&)>;
using HighWaterMarkCallback =
    std::function<void(const TCPConnectionPtr&, size_t)>;

// the data has been read to (buf, len)
using MessageCallback =
    std::function<void(const TCPConnectionPtr&, Buffer*, Timestamp)>;

void defaultConnectionCallback(const TCPConnectionPtr& conn);
void defaultMessageCallback(const TCPConnectionPtr& conn, Buffer* buffer,
                            Timestamp receiveTime);

///
/// @brief WeakCallback
///
/// @tparam CLASS
/// @tparam ARGS
///
template <typename CLASS, typename... ARGS>
class WeakCallback {
public:
    WeakCallback(const std::weak_ptr<CLASS>& object,
                 const std::function<void(CLASS*, ARGS...)>& function)
        : object_(object), function_(function) {}

    /// Default dtor, copy ctor and assignment are okay

    /// @brief
    /// @param ...args
    inline void operator()(ARGS&&... args) const {
        std::shared_ptr<CLASS> ptr(object_.lock());
        if (ptr) {
            function_(ptr.get(), std::forward<ARGS>(args)...);
        }
        // else
        // {
        //   LOG_TRACE << "expired";
        // }
    }

private:
    std::weak_ptr<CLASS> object_;
    std::function<void(CLASS*, ARGS...)> function_;
};

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(
    const std::shared_ptr<CLASS>& object, void (CLASS::*function)(ARGS...)) {
    return WeakCallback<CLASS, ARGS...>(object, function);
}

template <typename CLASS, typename... ARGS>
WeakCallback<CLASS, ARGS...> makeWeakCallback(
    const std::shared_ptr<CLASS>& object,
    void (CLASS::*function)(ARGS...) const) {
    return WeakCallback<CLASS, ARGS...>(object, function);
}
}  // namespace Lute
