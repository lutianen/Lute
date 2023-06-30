
#include <polaris/Buffer.h>

#define STR(x) #x
#define CHECK_EQUAL(x, y)                              \
    printf("%s %s:%d %s @ %s\n",                       \
           ((x) != (y)) ? ("[ " RED "Faild" CLR " ] ") \
                        : ("[ " GREEN "ok" CLR " ]"),  \
           __FILE__, __LINE__, STR(x), STR(y))

void output(Lute::Buffer&& buf, const void* inner) {
    Lute::Buffer newbuf(std::move(buf));
    // printf("New Buffer at %p, inner %p\n", &newbuf, newbuf.peek());
    CHECK_EQUAL(inner, newbuf.peek());
}

int main() {
    {
        Lute::Buffer buf;
        CHECK_EQUAL(buf.readableBytes(), 0);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);

        const std::string str(200, 'x');
        buf.append(str);
        CHECK_EQUAL(buf.readableBytes(), str.size());
        CHECK_EQUAL(buf.writableBytes(),
                    Lute::Buffer::kInitialSize - str.size());
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);

        const std::string str2 = buf.retrieveAsString(50);
        CHECK_EQUAL(str2.size(), 50);
        CHECK_EQUAL(buf.readableBytes(), str.size() - str2.size());
        CHECK_EQUAL(buf.writableBytes(),
                    Lute::Buffer::kInitialSize - str.size());
        CHECK_EQUAL(buf.prependableBytes(),
                    Lute::Buffer::kCheapPrepend + str2.size());
        CHECK_EQUAL(str2, std::string(50, 'x'));

        buf.append(str);
        CHECK_EQUAL(buf.readableBytes(), 2 * str.size() - str2.size());
        CHECK_EQUAL(buf.writableBytes(),
                    Lute::Buffer::kInitialSize - 2 * str.size());
        CHECK_EQUAL(buf.prependableBytes(),
                    Lute::Buffer::kCheapPrepend + str2.size());

        const std::string str3 = buf.retrieveAllAsString();
        CHECK_EQUAL(str3.size(), 350);
        CHECK_EQUAL(buf.readableBytes(), 0);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);
        CHECK_EQUAL(str3, std::string(350, 'x'));
    }

    {
        Lute::Buffer buf;
        buf.append(std::string(400, 'y'));
        CHECK_EQUAL(buf.readableBytes(), 400);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 400);

        buf.retrieve(50);
        CHECK_EQUAL(buf.readableBytes(), 350);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 400);

        buf.append(std::string(1000, 'z'));
        CHECK_EQUAL(buf.readableBytes(), 1350);
        CHECK_EQUAL(buf.writableBytes(), 0);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend + 50);

        buf.retrieveAll();
        CHECK_EQUAL(buf.readableBytes(), 0);
        CHECK_EQUAL(buf.writableBytes(), 1400);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);
    }

    {
        Lute::Buffer buf;
        buf.append(std::string(800, 'y'));
        CHECK_EQUAL(buf.readableBytes(), 800);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 800);

        buf.retrieve(500);
        CHECK_EQUAL(buf.readableBytes(), 300);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 800);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend + 500);

        buf.append(std::string(300, 'z'));
        CHECK_EQUAL(buf.readableBytes(), 600);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 600);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);
    }

    {
        Lute::Buffer buf;
        buf.append(std::string(2000, 'y'));
        CHECK_EQUAL(buf.readableBytes(), 2000);
        CHECK_EQUAL(buf.writableBytes(), 0);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);

        buf.retrieve(1500);
        CHECK_EQUAL(buf.readableBytes(), 500);
        CHECK_EQUAL(buf.writableBytes(), 0);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend + 1500);

        buf.shrink(0);
        CHECK_EQUAL(buf.readableBytes(), 500);
        CHECK_EQUAL(buf.writableBytes(), 1500);
        CHECK_EQUAL(buf.retrieveAllAsString(), std::string(500, 'y'));
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);
    }

    {
        Lute::Buffer buf;
        buf.append(std::string(200, 'y'));
        CHECK_EQUAL(buf.readableBytes(), 200);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 200);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend);

        int x = 0;
        buf.prepend(&x, sizeof x);
        CHECK_EQUAL(buf.readableBytes(), 204);
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize - 200);
        CHECK_EQUAL(buf.prependableBytes(), Lute::Buffer::kCheapPrepend - 4);
    }

    {
        Lute::Buffer buf;
        buf.append("LutePolaris");

        CHECK_EQUAL(buf.readableBytes(), 11);
        CHECK_EQUAL(buf.peekInt8(), 'L');
        CHECK_EQUAL(buf.peekUint8(), 'L');

        int top16 = buf.peekInt16();
        CHECK_EQUAL(top16, 'L' * 256 + 'u');
        top16 = buf.peekUint16();
        CHECK_EQUAL(top16, 'L' * 256 + 'u');

        CHECK_EQUAL(buf.peekInt32(),
                    'L' * 256 * 65536 + 'u' * 65536 + 't' * 256 + 'e');
        CHECK_EQUAL(buf.peekUint32(),
                    static_cast<uint32_t>('L' * 256 * 65536 + 'u' * 65536 +
                                          't' * 256 + 'e'));

        CHECK_EQUAL(buf.peekInt64(),
                    (static_cast<int64_t>('L' * 256 * 65536 + 'u' * 65536 +
                                          't' * 256 + 'e')
                     << 32) +
                        static_cast<int64_t>('P' * 256 * 65536 + 'o' * 65536 +
                                             'l' * 256 + 'a'));

        CHECK_EQUAL(buf.readInt8(), 'L');
        CHECK_EQUAL(buf.readUint8(), 'u');

        CHECK_EQUAL(buf.readInt16(), 't' * 256 + 'e');
        CHECK_EQUAL(buf.readUint16(), 'P' * 256 + 'o');

        CHECK_EQUAL(buf.readInt32(),
                    'l' * 256 * 65536 + 'a' * 65536 + 'r' * 256 + 'i');
        CHECK_EQUAL(buf.readableBytes(), 1);
        CHECK_EQUAL(buf.readUint8(), 's');
        std::string s = "LutePolarisLutePolar";
        buf.append(s.c_str(), s.size());

        CHECK_EQUAL(buf.readUint32(),
                    'L' * 256 * 65536 + 'u' * 65536 + 't' * 256 + 'e');
        CHECK_EQUAL(buf.readInt64(),
                    (static_cast<int64_t>('P' * 256 * 65536 + 'o' * 65536 +
                                          'l' * 256 + 'a')
                     << 32) +
                        static_cast<int64_t>('r' * 256 * 65536 + 'i' * 65536 +
                                             's' * 256 + 'L'));
        CHECK_EQUAL(buf.readUint64(),
                    (static_cast<uint64_t>('u' * 256 * 65536 + 't' * 65536 +
                                           'e' * 256 + 'P')
                     << 32) +
                        static_cast<uint64_t>('o' * 256 * 65536 + 'l' * 65536 +
                                              'a' * 256 + 'r'));
        CHECK_EQUAL(buf.writableBytes(), Lute::Buffer::kInitialSize);

        buf.appendInt8(-1);
        buf.appendInt16(-2);
        buf.appendInt32(-3);
        buf.appendInt64(-4);
        CHECK_EQUAL(buf.readableBytes(), 15);
        CHECK_EQUAL(buf.readInt8(), -1);
        CHECK_EQUAL(buf.readInt16(), -2);
        CHECK_EQUAL(buf.readInt32(), -3);
        CHECK_EQUAL(buf.readInt64(), -4);

        buf.appendUint8(12);
        buf.appendUint16(13);
        buf.appendUint32(14);
        buf.appendUint64(15);
        CHECK_EQUAL(buf.readableBytes(), 15);
        CHECK_EQUAL(buf.readUint8(), 12);
        CHECK_EQUAL(buf.readUint16(), 13);
        CHECK_EQUAL(buf.readUint32(), 14);
        CHECK_EQUAL(buf.readUint64(), 15);
    }

    {
        Lute::Buffer buf;
        buf.append(std::string(100000, 'x'));

        const char* null = NULL;
        CHECK_EQUAL(buf.findEOL(), null);
        CHECK_EQUAL(buf.findEOL(buf.peek() + 90000), null);
    }

    {
        Lute::Buffer buf;
        buf.append("Lute", 4);
        const void* inner = buf.peek();
        // printf("Buffer at %p, inner %p\n", &buf, inner);
        output(std::move(buf), inner);
    }
}
