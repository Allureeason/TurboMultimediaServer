#include "Acceptor.h"
#include "network/base/Network.h"

#include <cstring>
#include <unistd.h>

using namespace tmms::network;

Acceptor::Acceptor(EventLoop* loop, const InetAddress& addr)
    : Event(loop), addr_(addr) {
}

Acceptor::~Acceptor() {
    stop();
}

void Acceptor::setAcceptCallback(const AcceptCallback& cb) {
    accept_callback_ = cb;
}
void Acceptor::setAcceptCallback(AcceptCallback&& cb) {
    accept_callback_ = std::move(cb);
}

void Acceptor::open() {
    if (fd_ != -1) {
        ::close(fd_);
    }

    if (addr_.isIPv6()) {
        fd_ = SocketOpt::createNonblockingTcp(AF_INET6);
    } else {
        fd_ = SocketOpt::createNonblockingTcp(AF_INET);
    }

    if (fd_ == -1) {
        NETLOG_ERROR << "create socket failed err: " << errno << " err_msg: " << strerror(errno);
        return;
    }

    if (sockopt_) {
        delete sockopt_;
        sockopt_ = nullptr;
    }

    loop_->addEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));

    sockopt_ = new SocketOpt(fd_);
    sockopt_->setReuseAddr(true);
    sockopt_->setReusePort(true);

    sockopt_->bind(addr_);
    sockopt_->listen();

}


void Acceptor::start() {
    loop_->runInLoop([this]() {
        open();
    });
}

void Acceptor::stop() {
    loop_->removeEvent(std::dynamic_pointer_cast<Acceptor>(shared_from_this()));
}

void Acceptor::onRead() {
    while (true) {
        InetAddress peerAddr;
        int sock = sockopt_->accept(&peerAddr);
        if (sock >= 0) {
            
            if (accept_callback_) {
                accept_callback_(sock, peerAddr);
            }

        } else {
            int err = errno;
            if (err != EINTR && err != EAGAIN) {
                onClose();
                break;
            }
        }
    }
}

void Acceptor::onError(const std::string& msg) {
    NETLOG_ERROR << "acceptor error: " << msg;
    onClose();
}

void Acceptor::onClose() {
    stop();
    open();
}