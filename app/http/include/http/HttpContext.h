/**
 * @file HttpContext.h
 * @brief
 *
 * @author Lux
 */

#include <http/HttpRequest.h>
#include <polaris/Buffer.h>

namespace Lute {
namespace http {
class HttpContext {
public:
    enum class HttpRequestParseState {
        kExpectRequestLine,
        kExpectHeaders,
        kExpectBody,
        kGotAll,
    };

private:
    HttpRequestParseState state_;
    HttpRequest request_;

    bool processRequestLine(const char* begin, const char* end);

public:
    HttpContext() : state_(HttpRequestParseState::kExpectRequestLine) {}

    // default copy-ctor, dtor and assignment are fine

    // return false if any error
    bool parseRequest(Lute::Buffer* buf, Timestamp receiveTime);

    bool gotAll() const { return state_ == HttpRequestParseState::kGotAll; }

    void reset() {
        state_ = HttpRequestParseState::kExpectRequestLine;
        HttpRequest dummy;
        request_.swap(dummy);
    }

    const HttpRequest& request() const { return request_; }

    HttpRequest& request() { return request_; }
};
}  // namespace http
}  // namespace Lux
