#include <LuteBase.h>
#include <LutePolaris.h>
#include <unistd.h>

#include <cstdio>
#include <vector>

using namespace Lute;

int numThreads = 0;
class EchoClient;
std::vector<std::unique_ptr<EchoClient>> clients;
int current = 0;

class EchoClient {
private:
    EventLoop* loop_;
    TCPClient client_;

    void onConnection(const TCPConnectionPtr& conn) {
        LOG_INFO << conn->peerAddress().toIpPort() << " -> "
                 << conn->localAddress().toIpPort() << " is "
                 << (conn->connected() ? "up." : "down.");

        if (conn->connected()) {
            ++current;
            if (static_cast<decltype(clients.size())>(current) <
                clients.size()) {
                clients[current]->connect();
            }
            LOG_INFO << " *** connected " << current;
        }

        conn->send("world \n");
    }

    void onMessage(const TCPConnectionPtr& conn, Buffer* buf, Timestamp time) {
        std::string msg(buf->retrieveAllAsString());
        LOG_INFO << conn->name() << " recv " << msg.size() << " bytes at "
                 << time.toString();

        if (msg == "quit\n") {
            conn->send("bye\n");
            conn->shutdown();
        } else if (msg == "shutdown\n") {
            loop_->quit();
        } else {
            conn->send(msg);
        }
    }

public:
    EchoClient(EventLoop* loop, const InetAddress& listenAddr,
               const std::string& id)
        : loop_(loop), client_(loop, listenAddr, "EchoClient" + id) {
        client_.setConnectionCallback(
            std::bind(&EchoClient::onConnection, this, std::placeholders::_1));
        client_.setMessageCallback(
            std::bind(&EchoClient::onMessage, this, std::placeholders::_1,
                      std::placeholders::_2, std::placeholders::_3));
    }

    void connect() { client_.connect(); }
};

int main(int argc, char* argv[]) {
    LOG_INFO << "pid = " << getpid() << ", tid = " << CurrentThread::tid();
    if (argc > 1) {
        EventLoop loop;
        InetAddress serverAddr(argv[1], static_cast<uint16_t>(atoi(argv[2])));

        int n = 1;
        if (argc > 3) {
            n = atoi(argv[3]);
        }

        LOG_INFO << " n = " << n;

        clients.reserve(n);
        for (int i = 0; i < n; ++i) {
            char buf[32];
            snprintf(buf, sizeof buf, "%d", i + 1);
            clients.emplace_back(new EchoClient(&loop, serverAddr, buf));
        }

        clients[current]->connect();
        loop.loop();
    } else {
        printf("Usage: %s host_ip port [current#]\n", argv[0]);
    }
}