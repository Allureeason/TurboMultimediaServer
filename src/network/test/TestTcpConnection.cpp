#include "network/net/Acceptor.h"
#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"

using namespace tmms::network;

const std::string g_http_response = "HTTP/1.0 200 OK\r\nContent-Type: text/html; charset=utf-8\r\nServer: tmms/1.0\r\nContent-Length: 0\r\n\r\n";
std::vector<std::shared_ptr<Acceptor>> g_acceptors;

void test_tcp_connection(EventLoop* loop, std::vector<TcpConnectionPtr>& tcp_conns) {
    InetAddress server_addr("0.0.0.0:10000");
    std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, server_addr);


    acceptor->setAcceptCallback([server_addr, &loop, &tcp_conns](int sock, const InetAddress& addr) {
        NETLOG_INFO << "new connection from sock: " << sock << " addr: " << addr.toIpPort();
        auto tcpConn = std::make_shared<TcpConnection>(loop, sock, server_addr, addr);
        
        tcpConn->setCloseCallback([](TcpConnectionPtr conn) {
            NETLOG_INFO << "connection closed";
        });

        tcpConn->setRecvMessageCallback([](TcpConnectionPtr conn, MsgBuffer& msg) {
            std::string content = msg.retrieveAllAsString();
            NETLOG_INFO << "recv message from " << conn->getPeerAddr().toIpPort() << " message: " << content;
            conn->send(g_http_response.c_str(), g_http_response.size());
        });
        
        tcpConn->setWriteCompleteCallback([&loop](TcpConnectionPtr conn) {
            NETLOG_INFO << "write complete";
            loop->removeEvent(conn);
            conn->forceClose();
        });

        tcpConn->enableCheckIdleTime(3);

        tcp_conns.push_back(tcpConn);
        loop->addEvent(tcpConn);
    });
    

    acceptor->start();
    g_acceptors.push_back(acceptor);
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kInfo);

    auto cpu_num = 1;
    EventLoopThreadPool pool(cpu_num, 0, cpu_num);
    pool.start();

    std::vector<TcpConnectionPtr> tcp_conns;
    test_tcp_connection(pool.getNextLoop(), tcp_conns);

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // NETLOG_INFO << "tcp_conns size: " << tcp_conns.size();
    }

    return 0;
}