#include "TcpClient.h"
#include "network/net/SocketOpt.h"
#include "network/base/Network.h"

using namespace tmms::network;

TcpClient::TcpClient(EventLoop* loop, const InetAddress& serverAddr)
    : TcpConnection(loop, -1, InetAddress(), serverAddr)
    , server_addr_(serverAddr) {

}

TcpClient::~TcpClient() {
    onClose();
}

void TcpClient::setConnectionCallback(const ConnectionCallback& cb) {
    connection_cb_ = cb;
}

void TcpClient::setConnectionCallback(ConnectionCallback&& cb) {
    connection_cb_ = std::move(cb);
}

void TcpClient::connect() {
    loop_->runInLoop([this]() {
        connectInLoop();
    });
}

void TcpClient::connectInLoop() {
    loop_->assertInLoopThread();

    NETLOG_INFO << "connect to server: " << server_addr_.toIpPort();
    
    fd_ = SocketOpt::createNonblockingTcp(AF_INET);
    if (fd_ < 0) {
        NETLOG_ERROR << "create nonblocking tcp socket failed, error:" << errno;
        onClose();
        return;
    }

    NETLOG_INFO << "add event to loop, fd: " << fd_;
    status_ = kTcpConnStatusConnecting;
    loop_->addEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    enableWriting(true);

    SocketOpt sockOpt(fd_);
    int ret = sockOpt.connect(server_addr_);
    NETLOG_INFO << "connect to server: " << server_addr_.toIpPort() << " ret: " << ret;
    if (ret == 0) { // 连接成功
        updateConnectionStatus();
        return;
    } else if (ret == -1) { // 出错, 非阻塞连接
        int err = errno;
        if (err != EINPROGRESS) { // 不是正在连接，连接失败
            NETLOG_ERROR << "connect to server: " << server_addr_.toIpPort() << " failed, error:" << err << ", errmsg:" << strerror(err);
            onClose();
            return;
        } else {
            NETLOG_INFO << "connect to server: " << server_addr_.toIpPort() << " in progress";
        }
    }
}

void TcpClient::updateConnectionStatus() {
    status_ = kTcpConnStatusConnected;
    if (connection_cb_) {
        connection_cb_(std::dynamic_pointer_cast<TcpClient>(shared_from_this()), true);
    }
}

bool TcpClient::checkError() {
    int err = 0;
    socklen_t len = sizeof(err);
    ::getsockopt(fd_, SOL_SOCKET, SO_ERROR, &err, &len);
    return err != 0; 
}

void TcpClient::onRead() {
    if (status_ == kTcpConnStatusConnecting) {
        if (checkError()) {
            NETLOG_ERROR << "connect to server: " << server_addr_.toIpPort() << " failed, error:" << errno << ", errmsg:" << strerror(errno);
            onClose();
            return;
        }
        updateConnectionStatus();
        return;
    } else if (status_ == kTcpConnStatusConnected) {
        TcpConnection::onRead();
    }
}

void TcpClient::onWrite() {
    if (status_ == kTcpConnStatusConnecting) {
        if (checkError()) {
            NETLOG_ERROR << "connect to server: " << server_addr_.toIpPort() << " failed, error:" << errno << ", errmsg:" << strerror(errno);
            onClose();
            return;
        }
        updateConnectionStatus();
        return;
    }

    if (status_ == kTcpConnStatusConnected) {
        TcpConnection::onWrite();
    }
}

void TcpClient::onClose() {
    if (status_ == kTcpConnStatusConnecting || status_ == kTcpConnStatusConnected) {
    }
    status_ = kTcpConnStatusDisconnected;
    TcpConnection::onClose();
}

void TcpClient::send(std::list<BufferNodePtr>& vec) {
    if (status_ == kTcpConnStatusConnected) {
        TcpConnection::send(vec);
    } else {
        NETLOG_ERROR << "send data to server: " << server_addr_.toIpPort() << " failed, status: " << status_;
    }
}

void TcpClient::send(const void* buf, size_t len) {
    if (status_ == kTcpConnStatusConnected) {
        loop_->removeEvent(std::dynamic_pointer_cast<TcpClient>(shared_from_this()));
    }

    status_ = kTcpConnStatusConnected;
    TcpConnection::onClose();
}