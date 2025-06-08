#include "network/net/Acceptor.h"
#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"
#include "network/net/TcpConnection.h"
#include "network/net/EventLoopThreadPool.h"
#include "network/TcpServer.h"
#include "mmedia/rtmp/RtmpHandshake.h"

using namespace tmms::network;
using namespace tmms::mm;

std::shared_ptr<TcpServer> tcp_server;


void test_tcp_server(EventLoop* loop) {
    InetAddress server_addr("0.0.0.0:1935");
    tcp_server = std::make_shared<TcpServer>(loop, server_addr);

    tcp_server->setNewConnectionCallback([&loop](const TcpConnectionPtr& conn) {
        auto hsCtx = std::make_shared<RtmpHandShake>(conn);
        conn->setContext(kRtmpContext, hsCtx);
        hsCtx->start();
    });

    tcp_server->setMessageCallback([](const TcpConnectionPtr& conn, MsgBuffer& msg) {
        NETLOG_INFO << "recv message from " << conn->getPeerAddr().toIpPort() << " message size: " << msg.readableBytes();
        auto hsCtx = conn->getContext<RtmpHandShake>(kRtmpContext);
        auto ret = hsCtx->handshake(msg);
        if (ret == 0) {
            NETLOG_INFO << "handshake done";
        } else if (ret == 1) {
            NETLOG_INFO << "handshake not complete";
        } else {
            NETLOG_INFO << "handshake failed";
            conn->forceClose();
        }
    });

    tcp_server->setWriteCompleteCallback([&loop](const TcpConnectionPtr& conn) {
        auto hsCtx = conn->getContext<RtmpHandShake>(kRtmpContext);
        hsCtx->writeComplete();
    });

    tcp_server->setDestroyConnectionCallback([&loop](const TcpConnectionPtr& conn) {
        NETLOG_INFO << "destroy connection";
    });

    tcp_server->start();
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kTrace);

    auto cpu_num = 1;
    EventLoopThreadPool pool(cpu_num, 0, cpu_num);
    pool.start();

    test_tcp_server(pool.getNextLoop());

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        // tcp_server->dumpConnections();
    }

    return 0;
}