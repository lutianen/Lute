/**
 * @file Buffer.h
 * @brief A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
 */

#pragma once

#include <polaris/Sockets.h>

#include <algorithm>
#include <vector>

namespace Lute {

/// A buffer class modeled after org.jboss.netty.buffer.ChannelBuffer
///
/// @code
/// +-------------------+------------------+------------------+
/// | prependable bytes |  readable bytes  |  writable bytes  |
/// |                   |     (CONTENT)    |                  |
/// +-------------------+------------------+------------------+
/// |                   |                  |                  |
/// 0      <=      readerIndex   <=   writerIndex    <=     size
/// @endcode

class Buffer {
public:
    static const size_t kCheapPrepend = 8;
    static const size_t kInitialSize = 1024;

    inline explicit Buffer(size_t initialSize = kInitialSize)
        : buffer_(kCheapPrepend + initialSize),
          readerIndex_(kCheapPrepend),
          writerIndex_(kCheapPrepend) {
        assert(readableBytes() == 0);
        assert(writableBytes() == initialSize);
        assert(prependableBytes() == kCheapPrepend);
    }

    // implicit copy-ctor, move-ctor, dtor and assignment are fine
    // NOTE: implicit move-ctor is added in g++ 4.6

    inline size_t readableBytes() const { return writerIndex_ - readerIndex_; }

    inline size_t writableBytes() const {
        return buffer_.size() - writerIndex_;
    }

    inline size_t prependableBytes() const { return readerIndex_; }

    /// @brief Swap
    /// @param rhs
    inline void swap(Buffer& rhs) {
        buffer_.swap(rhs.buffer_);
        std::swap(readerIndex_, rhs.readerIndex_);
        std::swap(writerIndex_, rhs.writerIndex_);
    }

    inline const char* peek() const { return begin() + readerIndex_; }

    /// @brief Peek int64_t from network endian
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    inline int64_t peekInt64() const {
        assert(readableBytes() >= sizeof(int64_t));
        int64_t be64 = 0;
        ::memcpy(&be64, peek(), sizeof be64);
        assert(be64 >= 0);
        return static_cast<int64_t>(
            sockets::networkToHost64(static_cast<uint64_t>(be64)));
    }

    /// @brief Peek int32_t from network endian
    /// Require: buf->readableBytes() >= sizeof(int64_t)
    inline int32_t peekInt32() const {
        assert(readableBytes() >= sizeof(int32_t));
        int32_t be32 = 0;
        ::memcpy(&be32, peek(), sizeof be32);
        assert(be32 >= 0);
        return static_cast<int32_t>(
            sockets::networkToHost32(static_cast<uint32_t>(be32)));
    }

    inline int16_t peekInt16() const {
        assert(readableBytes() >= sizeof(int16_t));
        int16_t be16 = 0;
        ::memcpy(&be16, peek(), sizeof be16);
        assert(be16 >= 0);
        return static_cast<int16_t>(
            sockets::networkToHost16(static_cast<uint16_t>(be16)));
    }

    inline int8_t peekInt8() const {
        assert(readableBytes() >= sizeof(int8_t));
        int8_t x = *peek();
        return x;
    }

    inline char* beginWrite() { return begin() + writerIndex_; }
    inline const char* beginWrite() const { return begin() + writerIndex_; }

    inline const char* findCRLF() const {
        // const char* crlf = std::search(peek(), beginWrite(), kCRLF, kCRLF +
        // 2);
        /// XXX Use memmem() instead?
        const char* crlf = reinterpret_cast<const char*>(
            ::memmem(peek(), readableBytes(), kCRLF, 2));
        return crlf == beginWrite() ? nullptr : crlf;
    }

    inline const char* findCRLF(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        // const char* crlf = std::search(start, beginWrite(), kCRLF, kCRLF +
        // 2);
        /// XXX Use memmem() instead?
        const char* crlf = reinterpret_cast<const char*>(::memmem(
            start, static_cast<size_t>(beginWrite() - start), kCRLF, 2));
        return crlf == beginWrite() ? nullptr : crlf;
    }

    inline const char* findEOL() const {
        /// memchr 在参数 str 所指向的字符串的前 n
        /// 个字节中搜索第一次出现字符 c（一个无符号字符）的位置
        const void* eol = ::memchr(peek(), '\n', readableBytes());
        return reinterpret_cast<const char*>(eol);
    }

    inline const char* findEOL(const char* start) const {
        assert(peek() <= start);
        assert(start <= beginWrite());
        const void* eol =
            ::memchr(start, '\n', static_cast<size_t>(beginWrite() - start));
        return reinterpret_cast<const char*>(eol);
    }

    // retrieve returns void, to prevent
    // string str(retrieve(readableBytes()), readableBytes());
    // the evaluation of two functions are unspecified
    inline void retrieveAll() {
        readerIndex_ = kCheapPrepend;
        writerIndex_ = kCheapPrepend;
    }

    inline void retrieve(size_t len) {
        assert(len <= readableBytes());
        if (len < readableBytes())
            readerIndex_ += len;
        else
            retrieveAll();
    }

    inline void retrieveUntil(const char* end) {
        assert(peek() <= end);
        assert(end <= beginWrite());

        retrieve(static_cast<size_t>(end - peek()));
    }

    inline void retrieveInt64() { retrieve(sizeof(int64_t)); }
    inline void retrieveInt32() { retrieve(sizeof(int32_t)); }
    inline void retrieveInt16() { retrieve(sizeof(int16_t)); }
    inline void retrieveInt8() { retrieve(sizeof(int8_t)); }

    inline std::string retrieveAllAsString() {
        return retrieveAsString(readableBytes());
    }

    inline std::string retrieveAsString(size_t len) {
        assert(len <= readableBytes());
        std::string result(peek(), len);
        retrieve(len);
        return result;
    }

    inline std::string toStringPiece() const {
#if __cplusplus >= 201703L
        /// XXX std::string_view
        return std::string(peek(), readableBytes());
#else
        return std::string(peek(), static_cast<int>(readableBytes()));
#endif
    }

    inline void ensureWritableBytes(size_t len) {
        if (writableBytes() < len) makeSpace(len);
        assert(writableBytes() >= len);
    }

    inline void hasWritten(size_t len) {
        assert(len <= writableBytes());
        writerIndex_ += len;
    }

    inline void unwrite(size_t len) {
        assert(len <= readableBytes());
        writerIndex_ -= len;
    }

    inline void append(const char* data, size_t len) {
        ensureWritableBytes(len);
        std::copy(data, data + len, beginWrite());
        hasWritten(len);
    }

    // XXX std::string_view
    inline void append(const std::string& str) {
#if __cplusplus >= 201703L
        append(str.data(), str.size());
#else
        append(str.data(), static_cast<size_t>(str.size()));
#endif
    }

    inline void append(const void* data, size_t len) {
        append(static_cast<const char*>(data), len);
    }

    /// @brief Append int64_t using network endian
    inline void appendInt64(int64_t x) {
        int64_t be64 = static_cast<int64_t>(
            sockets::hostToNetwork64(static_cast<uint64_t>(x)));
        append(&be64, sizeof be64);
    }

    /// @brief Append int32_t using network endian
    inline void appendInt32(int32_t x) {
        int32_t be32 = static_cast<int32_t>(
            sockets::hostToNetwork32(static_cast<uint32_t>(x)));
        append(&be32, sizeof be32);
    }

    /// @brief  Append int16_t using network endian
    inline void appendInt16(int16_t x) {
        int16_t be16 = static_cast<int16_t>(
            sockets::hostToNetwork16(static_cast<uint16_t>(x)));
        append(&be16, sizeof be16);
    }

    /// @brief Append int8_t using network endian
    inline void appendInt8(int8_t x) { append(&x, sizeof x); }

    /// @brief Read int64_t from network endian
    /// Require: buf->readableBytes() >= sizeof(int32_t)
    inline int64_t readInt64() {
        int64_t result = peekInt64();
        retrieveInt64();
        return result;
    }

    inline int32_t readInt32() {
        int32_t result = peekInt32();
        retrieveInt32();
        return result;
    }

    inline int16_t readInt16() {
        int16_t result = peekInt16();
        retrieveInt16();
        return result;
    }

    inline int8_t readInt8() {
        int8_t result = peekInt8();
        retrieveInt8();
        return result;
    }

    inline void prepend(const void* data, size_t len) {
        assert(len <= prependableBytes());
        readerIndex_ -= len;
        const char* d = reinterpret_cast<const char*>(data);
        std::copy(d, d + len, begin() + readerIndex_);
    }

    /// @brief Prepend int64_t using network endian
    inline void prependInt64(int64_t x) {
        int64_t be64 = static_cast<int64_t>(
            sockets::hostToNetwork64(static_cast<uint64_t>(x)));
        prepend(&be64, sizeof be64);
    }

    inline void prependInt32(int32_t x) {
        int32_t be32 = static_cast<int32_t>(
            sockets::hostToNetwork32(static_cast<uint32_t>(x)));
        prepend(&be32, sizeof be32);
    }

    inline void prependInt16(int16_t x) {
        int16_t be16 = static_cast<int16_t>(
            sockets::hostToNetwork16(static_cast<uint16_t>(x)));
        prepend(&be16, sizeof be16);
    }

    inline void prependInt8(int8_t x) { prepend(&x, sizeof x); }

    /// XXX Test performance to do
    inline void shrink(size_t reserve) {
        buffer_.shrink_to_fit();
        ensureWritableBytes(readableBytes() + reserve);

        // // FIXME Use vector::shrink_to_fit() in c++ if possible
        // Buffer other;
        // other.ensureWritableBytes(readableBytes() + reserve);
        // other.append(toStringPiece());
        // swap(other);
    }

    inline size_t internalCapacity() const { return buffer_.capacity(); }

    /// Read data directly into buffer.
    ///
    /// It may implement with readv(2)
    /// @return result of read(2), @c errno is saved
    ssize_t readFd(int fd, int* savedErrno);

private:
    inline char* begin() { return &*buffer_.begin(); }
    inline const char* begin() const { return &*buffer_.begin(); }

    /// @brief 空间不足时，自动扩展，但 readerIndex_ 指向有问题，待修复
    /// @param len
    inline void makeSpace(size_t len) {
        if (writableBytes() + prependableBytes() < len + kCheapPrepend) {
            buffer_.resize(writerIndex_ + len);
        } else {
            // move readbale data to the front, make space inside buffer
            assert(kCheapPrepend < readerIndex_);
            size_t readable = readableBytes();
            std::copy(begin() + readerIndex_, begin() + writerIndex_,
                      begin() + kCheapPrepend);

            readerIndex_ = kCheapPrepend;
            writerIndex_ = readerIndex_ + readable;
            assert(readable == readableBytes());
        }
    }

private:
    std::vector<char> buffer_;
    size_t readerIndex_;
    size_t writerIndex_;

    static const char kCRLF[];
};

}  // namespace Lute
