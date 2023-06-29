/**
 * @file HttpServer.h
 * @brief
 *
 * @author Lux
 */

#pragma once

#include <LutePolaris.h>

#include <functional>

namespace Lute {
namespace http {

    class HttpResponse;
    class HttpRequest;

    /// A simple embeddable HTTP server designed for report status of a program.
    /// It is not a fully HTTP 1.1 compliant server, but provides minimum
    /// features that can communicate with HttpClient and Web browser. It is
    /// synchronous, just like Java Servlet.
    class HttpServer {
        HttpServer(const HttpServer&) = delete;
        HttpServer& operator=(HttpServer&) = delete;

    public:
        using HttpCallback =
            std::function<void(const HttpRequest&, HttpResponse*)>;

    private:
        TCPServer server_;
        HttpCallback httpCallback_;

    public:
        HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                   const std::string& name,
                   TCPServer::Option option = TCPServer::Option::kNoReusePort);

        EventLoop* getLoop() const { return server_.getLoop(); }

        // Not thread safe, callback be registered before calling start().
        void setHttpCallback(const HttpCallback& cb) { httpCallback_ = cb; }

        void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

        void start();

    private:
        void onConnection(const TCPConnectionPtr& conn);
        void onMessage(const TCPConnectionPtr& conn,
                       Buffer* buf, Timestamp);
        void onWriteCompleteCallback(const TCPConnectionPtr& conn);
        void onRequest(const TCPConnectionPtr& conn,
                       const HttpRequest&);
    };
}  // namespace http
}  // namespace Lute