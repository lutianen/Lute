/**
 * @file HttpRequest.h
 * @brief
 *
 * @author Lux
 */

#pragma once

#include <LuteBase.h>

#include <map>

namespace Lute {
namespace http {
class HttpRequest {
public:
    enum class Method { kInvalid, kGet, kPost, kHead, kPut, kDelete };
    enum class Version { kUnknown, kHttp10, kHttp11 };

private:
    Method method_;
    Version version_;

    std::string path_;
    std::string query_;
    Timestamp receiveTime_;
    std::map<std::string, std::string> headers_;
    std::string body_;

public:
    HttpRequest() : method_(Method::kInvalid), version_(Version::kUnknown) {}

    void setVersion(Version v) { version_ = v; }
    Version getVersion() const { return version_; }

    bool setMethod(const char* start, const char* end) {
#ifndef NDEBUG
        assert(method_ == Method::kInvalid);
#endif

        std::string m(start, end);
        if (m == "GET")
            method_ = Method::kGet;
        else if (m == "POST")
            method_ = Method::kPost;
        else if (m == "HEAD")
            method_ = Method::kHead;
        else if (m == "PUT")
            method_ = Method::kPut;
        else if (m == "DELETE")
            method_ = Method::kDelete;
        else
            method_ = Method::kInvalid;

        return method_ != Method::kInvalid;
    }
    Method method() const { return method_; }
    const char* methodString() const {
        const char* result = "UNKNOWN";
        switch (method_) {
            case Method::kGet:
                result = "GET";
                break;

            case Method::kPost:
                result = "GET";
                break;

            case Method::kHead:
                result = "GET";
                break;

            case Method::kPut:
                result = "GET";
                break;
            case Method::kDelete:
                result = "GET";
                break;
            default:
                break;
        }

        return result;
    }

    void setPath(const char* start, const char* end) {
        path_.assign(start, end);
    }
    const std::string& path() const { return path_; }

    void setQuery(const char* start, const char* end) {
        query_.assign(start, end);
    }
    const std::string& query() const { return query_; }

    void setReceiveTime(Timestamp t) { receiveTime_ = t; }
    Timestamp receiveTime() const { return receiveTime_; }

    // key: value
    void addHeader(const char* start, const char* colon, const char* end) {
        std::string field(start, colon);
        ++colon;
        while (colon < end && isspace(*colon)) ++colon;

        std::string value(colon, end);
        while (!value.empty() && isspace(value[value.size() - 1]))
            value.resize(value.size() - 1);
        headers_[field] = value;
    }
    std::string getHeader(const std::string& field) const {
        // string result;
        auto iter = headers_.find(field);
        // if (iter != headers_.end()) result = iter->second;
        // return result;

        return iter != headers_.end() ? iter->second : "";
    }
    const std::map<std::string, std::string>& headers() const { return headers_; }

    void setBody(const std::string& body) { body_ = body; }
    std::string body() const { return body_; }

    void swap(HttpRequest& that) {
        std::swap(method_, that.method_);
        std::swap(version_, that.version_);
        path_.swap(that.path_);
        query_.swap(that.query_);
        receiveTime_.swap(that.receiveTime_);
        headers_.swap(that.headers_);
        body_.swap(that.body_);
    }
};
}  // namespace http
}  // namespace Lux