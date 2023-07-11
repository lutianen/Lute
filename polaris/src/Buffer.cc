/**
 * @file Buffer.cc
 * @brief
 */

#include <errno.h>
#include <polaris/Buffer.h>
#include <polaris/Sockets.h>
#include <sys/uio.h>

using namespace Lute;

/// k - konstant
/// CRLF - Carriage Return Line Feed
const char Buffer::kCRLF[] = "\r\n";

const size_t Buffer::kCheapPrepend;
const size_t Buffer::kInitialSize;

///
/// @brief readv + extrabuf - 解决了缓冲区设置太大或太小的问题
/// @param fd
/// @param savedErrno
/// @return
///
ssize_t Buffer::readFd(int fd, int* savedErrno) {
    // saved an ioctl()/FIONREAD call to tell how much to read
    char extrabuf[65536];

    // Prepare iovecs
    struct iovec vec[2];
    const size_t writeable = writableBytes();
    vec[0].iov_base = begin() + writerIndex_;
    vec[0].iov_len = writeable;

    vec[1].iov_base = extrabuf;
    vec[1].iov_len = sizeof(extrabuf);

    /// When there is enough space in this buff, don't read into extrabuf
    /// When extrabuf is used, we read 128k-1 bytes at most.
    const int iovcnt = (writeable < sizeof(extrabuf)) ? 2 : 1;
    const ssize_t n = sockets::readv(fd, vec, iovcnt);
    if (n < 0) {
        *savedErrno = errno;
    } else if (static_cast<size_t>(n) <= writeable) {
        writerIndex_ += static_cast<size_t>(n);
    } else {
        writerIndex_ = buffer_.size();
        append(extrabuf, static_cast<size_t>(n) - writeable);
    }

    return n;
}
