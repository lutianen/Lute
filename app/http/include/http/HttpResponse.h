/**
 * @file HttpResponse.h
 * @brief
 *
 * @author Lux
 */

#pragma once

#include <LuteBase.h>
#include <LutePolaris.h>

#include <map>

namespace Lute {
namespace http {
class HttpResponse {
public:
    enum class HttpStatusCode {
        kUnknown,
        k200Ok = 200,
        k301MovedPermanently = 301,
        k400BadRequest = 400,
        k404NotFound = 404,
    };

private:
    std::map<std::string, std::string> headers_;
    HttpStatusCode statusCode_;
    // FIXME: add http version
    std::string statusMessage_;
    bool closeConnection_;
    std::string body_;

public:
    explicit HttpResponse(bool close)
        : statusCode_(HttpStatusCode::kUnknown), closeConnection_(close) {}

    void setStatusCode(HttpStatusCode code) { statusCode_ = code; }

    void setStatusMessage(const std::string& message) { statusMessage_ = message; }

    void setCloseConnection(bool on) { closeConnection_ = on; }

    bool closeConnection() const { return closeConnection_; }

    void setContentType(const std::string& contentType) {
        addHeader("Content-Type", contentType);
    }

    // FIXME: replace string with StringPiece
    void addHeader(const std::string& key, const std::string& value) {
        headers_[key] = value;
    }

    void setBody(const std::string& body) { body_ = body; }

    void appendToBuffer(Lute::Buffer* output) const;
};
}  // namespace http
}  // namespace Lux
