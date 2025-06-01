#include "UdpServer.h"
#include "network/base/Network.h"
#include "network/net/SocketOpt.h"

using namespace tmms::network;


UdpServer::UdpServer(EventLoop* loop, const InetAddress&  addr)
    : UdpSocket(loop, -1, addr, InetAddress()), addr_(addr) {
    
}

UdpServer::~UdpServer() {

}

void UdpServer::start() {
    loop_->runInLoop([this]() {
        open();
    });
}

void UdpServer::stop() {
    loop_->runInLoop([this]() {
        onClose();
    });
}

void UdpServer::open() {
    loop_->assertInLoopThread();

    fd_ = SocketOpt::createNonblockingUdp(AF_INET);
    if (fd_ < 0) {
        NETLOG_ERROR << "createNonblockingUdp failed, error: " << strerror(errno);
        return;
    }

    loop_->addEvent(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));

    SocketOpt opt(fd_);
    int ret = opt.bind(addr_);
    if (ret < 0) {
        NETLOG_ERROR << "udp socket bind failed, error: " << strerror(errno);
        return;
    }
}