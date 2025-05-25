#include "TcpConnection.h"
#include "network/base/Network.h"
#include <unistd.h>

using namespace tmms::network;

TcpConnection::TcpConnection(EventLoop* loop, int fd, const InetAddress& localAddr, const InetAddress& peerAddr)
    : Connection(loop, fd, localAddr, peerAddr) {
}

TcpConnection::~TcpConnection() {
    onClose();
}

void TcpConnection::setCloseCallback(const CloseConnectionCallback& cb) {
    close_cb_ = cb;
}

void TcpConnection::setCloseCallback(CloseConnectionCallback&& cb) {
    close_cb_ = std::move(cb);
}

void TcpConnection::onClose() {
    loop_->assertInLoopThread();

    loop_->runInLoop([this]() {
        if (close_cb_) {
            close_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()));
        }
        closed_ = true;
    });
}

void TcpConnection::forceClose() {
    loop_->runInLoop([this]() {
        onClose();
    });
}

void TcpConnection::setRecvMessageCallback(const MessageCallback& cb) {
    message_cb_ = cb;
}

void TcpConnection::setRecvMessageCallback(MessageCallback&& cb) {
    message_cb_ = std::move(cb);
}

void TcpConnection::onRead() {
    if (closed_) {
        NETLOG_ERROR << "tcp connection already closed";
        return;
    }

    while (true) {
        int err = 0;
        ssize_t n = message_buffer_.readFd(fd_, &err);
        if (n > 0) {
            if (message_cb_) {
                message_cb_(std::dynamic_pointer_cast<TcpConnection>(shared_from_this()), message_buffer_);
            }
        } else if (n == 0) {
            NETLOG_ERROR << "peer closed";
            onClose();
            break;
        } else {
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK) {
                NETLOG_ERROR << "read error: " << err << " - " << strerror(err);
                onClose();
            }
            break;
        }
    }
}

void TcpConnection::onError(const std::string& msg) {
    NETLOG_ERROR << "tcp connection error: " << msg;
    onClose();
}

void TcpConnection::setWriteCompleteCallback(const WriteCompleteCallback& cb) {
    write_complete_cb_ = cb;
}

void TcpConnection::setWriteCompleteCallback(WriteCompleteCallback&& cb) {
    write_complete_cb_ = std::move(cb);
}

void TcpConnection::send(const void * buf, size_t len) {
    if (closed_) {
        NETLOG_ERROR << "tcp connection already closed. peer: " << peerAddr_.toIpPort();
        return;
    }

    if (loop_->isInLoopThread()) {
        sendInLoop(buf, len);
    } else {
        loop_->runInLoop([this, buf, len]() {
            sendInLoop(buf, len);
        });
    }
}

void TcpConnection::send(const std::list<BufferNodePtr>& vec) {
    if (closed_) {
        NETLOG_ERROR << "tcp connection already closed. peer: " << peerAddr_.toIpPort();
        return;
    }

    if (loop_->isInLoopThread()) {
        sendInLoop(vec);
    } else {
        loop_->runInLoop([this, vec]() {
            sendInLoop(vec);
        });
    }
}

void TcpConnection::sendInLoop(const void * buf, size_t len) {
    if (closed_) {
        NETLOG_ERROR << "tcp connection already closed. peer: " << peerAddr_.toIpPort();
        return;
    }
    size_t size = len;
    size_t sendLen = 0;
    if (io_vec_list_.empty()) {
        sendLen = ::write(fd_, buf, len);
        if (sendLen < 0) {
            int err = errno;
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK) {
                NETLOG_ERROR << "read error: " << err << " - " << strerror(err);
                onClose();
                return;
            }
        }

        size -= sendLen;
    }


    if (size > 0) {
        struct iovec iov;
        iov.iov_base = const_cast<void*>(buf + sendLen);
        iov.iov_len = size;
        io_vec_list_.push_back(iov);

        enableWriting(true);
    }
}

void TcpConnection::sendInLoop(const std::list<BufferNodePtr>& vec) {
    if (closed_) {
        NETLOG_ERROR << "tcp connection already closed. peer: " << peerAddr_.toIpPort();
        return;
    }

    for (const auto& node : vec) {
        struct iovec iov;
        iov.iov_base = node->buf;
        iov.iov_len = node->len;
        io_vec_list_.push_back(iov);
    }

    if (!io_vec_list_.empty()) {
        enableWriting(true);
    }
}