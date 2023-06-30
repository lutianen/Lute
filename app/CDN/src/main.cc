#include <LuteBase.h>
#include <LutePolaris.h>

class CDN {
public:
    CDN(Lute::EventLoop* loop, const Lute::InetAddress& listenAddr,
        const std::string& name,
        Lute::TCPServer::Option option = Lute::TCPServer::Option::kNoReusePort)
        : server_(loop, listenAddr, name, option),
          recvDataFile_("./recv.data") {
        server_.setConnectionCallback(
            std::bind(&CDN::onConnection, this, std::placeholders::_1));

        server_.setMessageCallback(
            std::bind(&CDN::onMessage, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
    }

    void start() {
        LOG_INFO << server_.name() << " start ...";
        server_.start();
    }

    void setThreadNum(int numThreads) { server_.setThreadNum(numThreads); }

private:
    Lute::TCPServer server_;
    Lute::AppendFile recvDataFile_;

    void onConnection(const Lute::TCPConnectionPtr& conn) {
        if (conn->connected()) {
            const char shakeHand[] = "\xDF\xEF\xFF";
            conn->send(shakeHand, sizeof(shakeHand) - 1);
        }
    }
    void onMessage(const Lute::TCPConnectionPtr& conn, Lute::Buffer* buf,
                   Lute::Timestamp) {
        if (conn->connected()) {
            auto len = buf->readableBytes();
            for (size_t i = 0; i < len / 2; ++i) {
                const auto c = buf->readUint16();
                printf("%04x\n", c);
                recvDataFile_.append(reinterpret_cast<const char*>(&c), 1);
            }
            recvDataFile_.flush();
        }
    }
};

int main() {
    Lute::EventLoop loop;
    CDN cdn(&loop, Lute::InetAddress("127.0.0.1", 5836), "CDN");

    cdn.start();

    loop.loop();
}