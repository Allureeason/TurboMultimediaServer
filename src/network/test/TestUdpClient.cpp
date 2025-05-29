#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/UdpClient.h"

using namespace tmms::network;

const std::string g_http_request = "GET / HTTP/1.0\r\nHost: 192.168.1.106:10000\r\nUser-Agent: curl/8.1.0\r\nAccept: */*\r\nContent-Length: 0\r\n\r\n";
std::vector<std::shared_ptr<UdpClient>> g_clients;

void test_udp_client(EventLoop* loop) {
    InetAddress server_addr("192.168.1.106:10000");
    struct sockaddr_in6 sock_addr;
    sock_addr.sin6_family = AF_INET6;
    sock_addr.sin6_port = htons(10000);
    inet_pton(AF_INET6, "192.168.1.106", &sock_addr.sin6_addr);



    std::shared_ptr<UdpClient> client = std::make_shared<UdpClient>(loop, server_addr);
    client->setConnectionCallback([](const UdpSocketPtr& conn, bool isConnected) {
        if (isConnected) {
            NETLOG_INFO << "connect to server: " << conn->getPeerAddr().toIpPort() << " success";
            auto client = std::dynamic_pointer_cast<UdpClient>(conn);
            client->send(g_http_request.c_str(), g_http_request.size());
        } else {
            NETLOG_INFO << "connect to server: " << conn->getPeerAddr().toIpPort() << " failed";
        }
    });

    client->setMessageCallback([](const InetAddress& addr, MsgBuffer& msg) {
        std::string content = msg.retrieveAllAsString();
        NETLOG_INFO << "recv message from " << addr.toIpPort() << " message: " << content;
    });

    client->setCloseCallback([](const UdpSocketPtr& conn) {
        NETLOG_INFO << "connection closed";
    });

    client->setWriteCompleteCallback([](const UdpSocketPtr& conn) {
        NETLOG_INFO << "write complete";
        // conn->forceClose();
    });

    client->connect();
    g_clients.push_back(client);
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kInfo);
    EventLoopThreadPool pool(1, 0, 1);
    pool.start();

    test_udp_client(pool.getNextLoop());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // NETLOG_INFO << "tcp_conns size: " << tcp_conns.size();
    }

    return 0;
}