/**
 * @file HttpContext.cc
 * @brief
 *
 * @author Lux
 */

#include <http/HttpContext.h>

using namespace Lute;

// HTTP/1.1
// GET /hello HTTP/1.1
// Accept: text/html,application/xhtml+xml,application/xml;
// Accept-Encoding: gzip, deflate
// Accept-Language: zh-CN,zh;q=0.9,en;q=0.8
// Cache-Control: max-age=0
// Connection: keep-alive
// Host: 192.168.132.128:8000
// Upgrade-Insecure-Requests: 1
// User-Agent: ...

bool http::HttpContext::processRequestLine(const char* begin, const char* end) {
    bool succeed = false;
    const char* start = begin;
    const char* space = std::find(start, end, ' ');
    if (space != end && request_.setMethod(start, space)) {
        start = space + 1;
        space = std::find(start, end, ' ');
        if (space != end) {
            const char* question = std::find(start, space, '?');
            if (question != space) {
                request_.setPath(start, question);
                request_.setQuery(question, space);
            } else {
                request_.setPath(start, space);
            }
            start = space + 1;
            succeed = end - start == 8 && std::equal(start, end - 1, "HTTP/1.");
            if (succeed) {
                if (*(end - 1) == '1') {
                    request_.setVersion(HttpRequest::Version::kHttp11);
                } else if (*(end - 1) == '0') {
                    request_.setVersion(HttpRequest::Version::kHttp10);
                } else {
                    succeed = false;
                }
            }
        }
    }
    return succeed;
}

// return false if any error
bool http::HttpContext::parseRequest(Buffer* buf,
                                     Timestamp receiveTime) {
    bool ok = true;
    bool hasMore = true;
    while (hasMore) {
        if (state_ == HttpRequestParseState::kExpectRequestLine) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                ok = processRequestLine(buf->peek(), crlf);
                if (ok) {
                    request_.setReceiveTime(receiveTime);
                    buf->retrieveUntil(crlf + 2);
                    state_ = HttpRequestParseState::kExpectHeaders;
                } else {
                    hasMore = false;
                }
            } else {
                hasMore = false;
            }
        } else if (state_ == HttpRequestParseState::kExpectHeaders) {
            const char* crlf = buf->findCRLF();
            if (crlf) {
                const char* colon = std::find(buf->peek(), crlf, ':');
                if (colon != crlf) {
                    request_.addHeader(buf->peek(), colon, crlf);
                } else {
                    // empty line, end of header
                    state_ = HttpRequestParseState::kExpectBody;
                }
                buf->retrieveUntil(crlf + 2);
            } else {
                hasMore = false;
            }
        } else if (state_ == HttpRequestParseState::kExpectBody) {
            std::string body = buf->retrieveAllAsString();
            if (!body.empty()) request_.setBody(body);

            state_ = HttpRequestParseState::kGotAll;
            hasMore = false;
        }
    }
    return ok;
}
