#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/UdpServer.h"

using namespace tmms::network;

const std::string g_http_request = "GET / HTTP/1.0\r\nHost: 192.168.1.106:10000\r\nUser-Agent: curl/8.1.0\r\nAccept: */*\r\nContent-Length: 0\r\n\r\n";
std::vector<std::shared_ptr<UdpServer>> g_servers;

void test_udp_server(EventLoop* loop) {
    InetAddress server_addr("192.168.1.106:10000");
    std::shared_ptr<UdpServer> server = std::make_shared<UdpServer>(loop, server_addr);

    server->setMessageCallback([server](const InetAddress& addr, MsgBuffer& msg) {
        std::string content = msg.retrieveAllAsString();
        NETLOG_INFO << "recv message from " << addr.toIpPort() << " message: " << content;
        struct sockaddr_in6 sock_addr;
        addr.getSockAddress((struct sockaddr*)&sock_addr);
        server->send(content.c_str(), content.size(), (sockaddr*)&sock_addr, sizeof(sock_addr));
    });

    server->setWriteCompleteCallback([](const UdpSocketPtr& conn) {
        NETLOG_INFO << "write complete";
    });

    server->start();
    g_servers.push_back(server);
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kInfo);
    EventLoopThreadPool pool(1, 0, 1);
    pool.start();

    test_udp_server(pool.getNextLoop());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // NETLOG_INFO << "tcp_conns size: " << tcp_conns.size();
    }

    return 0;
}