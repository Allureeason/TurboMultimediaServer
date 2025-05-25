#include "Connection.h"

using namespace tmms::network;

Connection::Connection(EventLoop* loop, int fd, const InetAddress& localAddr, const InetAddress& peerAddr)
    : Event(loop, fd), localAddr_(localAddr), peerAddr_(peerAddr) {
}

void Connection::setPeerAddr(const InetAddress& addr) {
    peerAddr_ = addr;
}

void Connection::setLocalAddr(const InetAddress& addr) {
    localAddr_ = addr;
}

const InetAddress& Connection::getPeerAddr() const {
    return peerAddr_;
}

const InetAddress& Connection::getLocalAddr() const {
    return localAddr_;
}

void Connection::setContext(int type, const ContextPtr& ctx) {
    contexts_[type] = ctx;
}

void Connection::setContext(int type, ContextPtr&& ctx) {
    contexts_[type] = std::move(ctx);
}

void Connection::removeContext(int type) {
    contexts_[type].reset();
}

void Connection::clearContexts() {
    contexts_.clear();
}

void Connection::setActiveCallback(const ActiveCallback& cb) {
    active_cb_ = cb;
}

void Connection::setActiveCallback(ActiveCallback&& cb) {
    active_cb_ = std::move(cb);
}

void Connection::active() {
    if (!active_.load()) {
        loop_->runInLoop([this]() {
            active_ = true;
            if (active_cb_) {
                active_cb_(std::dynamic_pointer_cast<Connection>(shared_from_this()));
            }
        });
    }
}

void Connection::deactive() {
    active_.store(false);
}