#include "TcpServer.h"
#include "network/base/Network.h"

using namespace tmms::network;

TcpServer::TcpServer(EventLoop* loop, const InetAddress& addr)
    : loop_(loop)
    , addr_(addr)
    , acceptor_(std::make_shared<Acceptor>(loop, addr)) {

}

TcpServer::~TcpServer() {
}

void TcpServer::start() {
    acceptor_->setAcceptCallback(std::bind(&TcpServer::onAccept, this, std::placeholders::_1, std::placeholders::_2));
    acceptor_->start();
}

void TcpServer::stop() {
    acceptor_->stop();
}

void TcpServer::setNewConnectionCallback(const NewConnectionCallback& cb) {
    new_connection_cb_ = cb;
}

void TcpServer::setNewConnectionCallback(NewConnectionCallback&& cb) {
    new_connection_cb_ = std::move(cb);
}

void TcpServer::setMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
}

void TcpServer::setMessageCallback(MessageCallback&& cb) {
    message_cb_ = std::move(cb);
}

void TcpServer::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_cb_ = cb;
}

void TcpServer::setWriteCompleteCallback(WriteCompleteCallback&& cb) {
    write_complete_cb_ = std::move(cb);
}

void TcpServer::setActiveCallback(const ActiveCallback& cb) {
    active_cb_ = cb;
}

void TcpServer::setActiveCallback(ActiveCallback&& cb) {
    active_cb_ = std::move(cb);
}

void TcpServer::setDestroyConnectionCallback(const DestroyConnectionCallback& cb) {
    destroy_connection_cb_ = cb;
}

void TcpServer::setDestroyConnectionCallback(DestroyConnectionCallback&& cb) {
    destroy_connection_cb_ = std::move(cb);
}

void TcpServer::onAccept(int sockfd, const InetAddress& peerAddr) {
    NETLOG_INFO << "new connection from " << peerAddr.toIpPort() << " fd: " << sockfd;
    auto conn = std::make_shared<TcpConnection>(loop_, sockfd, InetAddress(), peerAddr);
    // 设置连接关闭回调
    conn->setCloseCallback(std::bind(&TcpServer::onConnectionClose, this, std::placeholders::_1));
    // 设置消息回调
    conn->setRecvMessageCallback(message_cb_);
    // 设置写完成回调
    conn->setWriteCompleteCallback(write_complete_cb_);
    // 设置活跃回调
    conn->setActiveCallback(active_cb_);
    // 添加到连接集合
    connections_.insert(conn);
    // 添加连接到事件循环
    loop_->addEvent(conn);
    conn->enableCheckIdleTime(30);
    NETLOG_TRACE << "conn fd:" << sockfd << "conn use count: " << conn.use_count();
    // 连接成功，通知上层
    if (new_connection_cb_) {
        new_connection_cb_(conn);
    }
}

void TcpServer::onConnectionClose(const TcpConnectionPtr& conn) {
    NETLOG_INFO << "host: " << conn->getLocalAddr().toIpPort() << " connection close fd: " << conn->getFd();
    loop_->assertInLoopThread();

    connections_.erase(conn);
    loop_->removeEvent(conn);

    if (destroy_connection_cb_) {
        destroy_connection_cb_(conn);
    }
}

void TcpServer::dumpConnections() const {
    NETLOG_INFO << "connections size: " << connections_.size();
    for (const auto& conn : connections_) {
        NETLOG_INFO << "connection fd: " << conn->getFd() << " peer addr: " << conn->getPeerAddr().toIpPort();
    }
}