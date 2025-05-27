#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/net/TcpClient.h"

using namespace tmms::network;

const std::string g_http_request = "GET / HTTP/1.0\r\nHost: 192.168.1.106:10000\r\nUser-Agent: curl/8.1.0\r\nAccept: */*\r\nContent-Length: 0\r\n\r\n";
std::vector<std::shared_ptr<TcpClient>> g_clients;

void test_tcp_connection(EventLoop* loop) {
    InetAddress server_addr("192.168.1.106:10000");
    std::shared_ptr<TcpClient> client = std::make_shared<TcpClient>(loop, server_addr);
    client->setConnectionCallback([](const TcpConnectionPtr& conn, bool isConnected) {
        if (isConnected) {
            NETLOG_INFO << "connect to server: " << conn->getPeerAddr().toIpPort() << " success";
            conn->send(g_http_request.c_str(), g_http_request.size());
        } else {
            NETLOG_INFO << "connect to server: " << conn->getPeerAddr().toIpPort() << " failed";
        }
    });

    client->setRecvMessageCallback([](const TcpConnectionPtr& conn, MsgBuffer& msg) {
        std::string content = msg.retrieveAllAsString();
        NETLOG_INFO << "recv message from " << conn->getPeerAddr().toIpPort() << " message: " << content;
    });

    client->setWriteCompleteCallback([](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "write complete";
        conn->forceClose();
    });

    client->setCloseCallback([](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "connection closed";
    });

    client->connect();
    g_clients.push_back(client);
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kInfo);
    EventLoopThreadPool pool(1, 0, 1);
    pool.start();

    test_tcp_connection(pool.getNextLoop());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // NETLOG_INFO << "tcp_conns size: " << tcp_conns.size();
    }

    return 0;
}