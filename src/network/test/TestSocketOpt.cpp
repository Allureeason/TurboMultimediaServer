#include "network/net/SocketOpt.h"
#include "network/net/InetAddress.h"
#include "network/base/Network.h"
#include <sys/socket.h>
#include <unistd.h>
#include <cstring>

using namespace tmms::network;

void test_client() {
    int sock = SocketOpt::createNonblockingTcp(AF_INET);
    if (sock == -1) {
        NETLOG_ERROR << "create socket failed";
        return;
    }

    InetAddress addr("192.168.1.101:10000");
    SocketOpt sock_opt(sock);
    sock_opt.setNonBlocking(false);

    if (sock_opt.connect(addr) == -1) {
        NETLOG_ERROR << "connect failed, errno: " << errno << ", errstr: " << strerror(errno);
        return;
    }

    NETLOG_INFO << "connect success";
    // 打印socket option 连接信息
    NETLOG_INFO << "sock: " << sock_opt.getSock();
    NETLOG_INFO << "PeerAddr: " << sock_opt.getPeerAddr()->toIpPort();
    NETLOG_INFO << "LocalAddr: " << sock_opt.getLocalAddr()->toIpPort();
}

void test_server() {
    int sock = SocketOpt::createNonblockingTcp(AF_INET);
    if (sock == -1) {
        NETLOG_ERROR << "create socket failed";
        return;
    }

    // 创建socket option
    SocketOpt sock_opt(sock);
    sock_opt.setNonBlocking(false);

    // 绑定地址
    InetAddress addr("192.168.1.101:10000");
    sock_opt.bind(addr);
    sock_opt.listen();

    // 接受连接
    InetAddress peer_addr;
    int conn_sock = sock_opt.accept(&peer_addr);
    if (conn_sock == -1) {
        NETLOG_ERROR << "accept failed, errno: " << errno << ", errstr: " << strerror(errno);
        return;
    }

    // 打印连接信息
    NETLOG_INFO << "accept success";
    NETLOG_INFO << "PeerAddr: " << peer_addr.toIpPort();
    NETLOG_INFO << "Sock: " << conn_sock;

    // 关闭连接
    ::close(sock);
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);


    // test_client();

    test_server();


    return 0;
}