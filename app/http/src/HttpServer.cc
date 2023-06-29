/**
 * @file HttpServer.cc
 * @brief
 *
 * @author Lux
 */

#include <LuteBase.h>
#include <http/HttpContext.h>
#include <http/HttpRequest.h>
#include <http/HttpResponse.h>
#include <http/HttpServer.h>

using namespace Lute;
using namespace Lute::http;

namespace Lute {
namespace http {
    namespace detail {
        void defaultHttpCallback(const HttpRequest&, HttpResponse* resp) {
            resp->setStatusCode(HttpResponse::HttpStatusCode::k404NotFound);
            resp->setStatusMessage("Not Found");
            resp->setCloseConnection(true);
        }
    }  // namespace detail
}  // namespace http
}  // namespace Lute

HttpServer::HttpServer(EventLoop* loop, const InetAddress& listenAddr,
                       const std::string& name, TCPServer::Option option)
    : server_(loop, listenAddr, name, option),
      httpCallback_(detail::defaultHttpCallback) {
    server_.setConnectionCallback(
        std::bind(&HttpServer::onConnection, this, std::placeholders::_1));
    server_.setMessageCallback(
        std::bind(&HttpServer::onMessage, this, std::placeholders::_1,
                  std::placeholders::_2, std::placeholders::_3));
}

void HttpServer::start() {
    LOG_WARN << "HttpServer[" << server_.name() << "] starts listening on "
             << server_.ipPort();
    server_.start();
}

void HttpServer::onConnection(const TCPConnectionPtr& conn) {
    if (conn->connected()) {
        conn->setContext(HttpContext());
    }
}

void HttpServer::onMessage(const TCPConnectionPtr& conn, Buffer* buf,
                           Timestamp receiveTime) {
    HttpContext* context =
        Lute::any_cast<HttpContext>(conn->getMutableContext());

    // parse request
    if (!context->parseRequest(buf, receiveTime)) {
        conn->send("HTTP/1.1 400 Bad Request\r\n\r\n");
        conn->shutdown();
    }

    if (context->gotAll()) {
        onRequest(conn, context->request());
        context->reset();
    }
}

void HttpServer::onRequest(const TCPConnectionPtr& conn,
                           const HttpRequest& req) {
    const std::string& connection = req.getHeader("Connection");
    bool close = connection == "close" ||
                 (req.getVersion() == HttpRequest::Version::kHttp10 &&
                  connection != "Keep-Alive");
    HttpResponse response(close);
    httpCallback_(req, &response);
    Buffer buf;
    response.appendToBuffer(&buf);
    conn->send(&buf);
    if (response.closeConnection()) {
        conn->shutdown();
    }
}
