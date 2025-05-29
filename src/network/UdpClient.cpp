#include "UdpClient.h"
#include "network/net/SocketOpt.h"
#include "network/base/Network.h"

using namespace tmms::network;

UdpClient::UdpClient(EventLoop* loop, const InetAddress& server_addr)
    : UdpSocket(loop, -1, InetAddress(), server_addr), server_addr_(server_addr) {
}

UdpClient::~UdpClient() {
}

void UdpClient::connect() {
    loop_->runInLoop([this]() {
        connnectInLoop();
    });
}

void UdpClient::setConnectionCallback(const UdpConnectionCallback& cb) {
    connection_cb_ = cb;
}

void UdpClient::setConnectionCallback(UdpConnectionCallback&& cb) {
    connection_cb_ = std::move(cb);
}

void UdpClient::send(const void* buf, size_t len) {
    UdpSocket::send(buf, len, (struct sockaddr*)&sock_addr_, sock_addr_len_);
}

void UdpClient::send(const std::list<UdpBufferNodePtr>& list) {
    UdpSocket::send(list, (struct sockaddr*)&sock_addr_, sock_addr_len_);
}

void UdpClient::onClose() {
    if (connected_) {
        loop_->removeEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        connected_ = false;
        UdpSocket::onClose();
    }
}

void UdpClient::connnectInLoop() {
    loop_->assertInLoopThread();

    fd_ = SocketOpt::createNonblockingUdp(AF_INET);
    if (fd_ < 0) {
        NETLOG_ERROR << "create udp socket failed";
        return;
    }

    connected_ = true;
    loop_->addEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
    SocketOpt opt(fd_);
    server_addr_.getSockAddress((struct sockaddr*)&sock_addr_);
    NETLOG_INFO << "udp client connect to server: " << SocketOpt::dumpSockAddr((struct sockaddr*)&sock_addr_);

    if (connection_cb_) {
        connection_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()), true);
    }
}