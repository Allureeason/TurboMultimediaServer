#include "network/net/Acceptor.h"
#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/TcpServer.h"

using namespace tmms::network;

const std::string g_http_response = "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nServer: tmms/1.0\r\nContent-Length: 0\r\n\r\n";
std::shared_ptr<TcpServer> tcp_server;


void test_tcp_server(EventLoop* loop) {
    InetAddress server_addr("0.0.0.0:10000");
    tcp_server = std::make_shared<TcpServer>(loop, server_addr);


    tcp_server->setNewConnectionCallback([&loop](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "new connection from " << conn->getPeerAddr().toIpPort();
    });

    tcp_server->setMessageCallback([](const TcpConnectionPtr& conn, MsgBuffer& msg) {
        std::string content = msg.retrieveAllAsString();
        NETLOG_INFO << "recv message from " << conn->getPeerAddr().toIpPort() << " message: " << content;
        conn->send(g_http_response.c_str(), g_http_response.size());
    });

    tcp_server->setWriteCompleteCallback([&loop](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "write complete";
        conn->forceClose();
    });

    tcp_server->setDestroyConnectionCallback([&loop](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "destroy connection";
    });

    tcp_server->start();
}


int main() {
    // tmms::base::g_logger = new tmms::base::Logger(nullptr);
    // tmms::base::g_logger->setLevel(tmms::base::LogLevel::kInfo);

    auto cpu_num = std::thread::hardware_concurrency() * 2 + 1;
    EventLoopThreadPool pool(cpu_num, 0, cpu_num);
    pool.start();

    test_tcp_server(pool.getNextLoop());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(5));
        tcp_server->dumpConnections();
    }

    return 0;
}