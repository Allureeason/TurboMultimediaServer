#include "UdpSocket.h"
#include "network/base/Network.h"

using namespace tmms::network;

UdpSocket::UdpSocket(EventLoop* loop, int sockfd, const InetAddress& local_addr, const InetAddress& peer_addr)
    : Connection(loop, sockfd, local_addr, peer_addr) {
}

UdpSocket::~UdpSocket() {
}

void UdpSocket::setMessageCallback(const UdpSocketMessageCallback& cb) {
    message_cb_ = cb;
}

void UdpSocket::setMessageCallback(UdpSocketMessageCallback&& cb) {
    message_cb_ = std::move(cb);
}

void UdpSocket::setWriteCompleteCallback(const UdpSocketWriteCompleteCallback& cb) {
    write_complete_cb_ = cb;
}

void UdpSocket::setWriteCompleteCallback(UdpSocketWriteCompleteCallback&& cb) {
    write_complete_cb_ = std::move(cb);
}

void UdpSocket::setCloseCallback(const UdpSocketCloseCallback& cb) {
    close_cb_ = cb;
}

void UdpSocket::setCloseCallback(UdpSocketCloseCallback&& cb) {
    close_cb_ = std::move(cb);
}

void UdpSocket::onRead() {
    loop_->assertInLoopThread();

    if (closed_) {
        NETLOG_ERROR << "udp socket is closed, fd: " << fd_ << ", peer: " << peerAddr_.toIpPort();
        return;
    }

    // 延长udp socket生命周期
    extendLife();

    while (true) {
        struct sockaddr_in6 peer_addr;
        socklen_t peer_addr_len = sizeof(peer_addr);
        int32_t n = ::recvfrom(fd_, message_buffer_.beginWrite(), message_buffer_size_, 0, (struct sockaddr*)&peer_addr, &peer_addr_len);
        if (n > 0) {
            InetAddress peer;
            if (peer_addr.sin6_family == AF_INET6) {
                char ip[INET6_ADDRSTRLEN] = {0};
                ::inet_ntop(AF_INET6, &peer_addr.sin6_addr, ip, sizeof(ip));
                peer.setIP(ip);
                peer.setPort(ntohs(peer_addr.sin6_port));
                peer.setIsIPv6(true);
            } else {
                char ip[INET_ADDRSTRLEN] = {0};
                struct sockaddr_in* peer_addr4 = reinterpret_cast<struct sockaddr_in*>(&peer_addr);
                ::inet_ntop(AF_INET, &peer_addr4->sin_addr, ip, sizeof(ip));
                peer.setIP(ip);
                peer.setPort(ntohs(peer_addr4->sin_port));
                peer.setIsIPv6(false);
            }

            message_buffer_.hasWritten(n);
            if (message_cb_) {
                message_cb_(peer, message_buffer_);
            }

            message_buffer_.retrieveAll();
        } else if (n < 0) {
            int err = errno;
            if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK) {
                NETLOG_ERROR << "udp socket recvfrom error, fd: " << fd_ << ", error: " << strerror(err);
                onClose();
                return;
            }
            break;
        }
    }
}

void UdpSocket::onWrite() {
    loop_->assertInLoopThread();

    if (closed_) {
        NETLOG_ERROR << "udp socket is closed, fd: " << fd_ << ", peer: " << peerAddr_.toIpPort();
        return;
    }

    // 延长udp socket生命周期
    extendLife();

    if (!buffer_list_.empty()) {
        while (true) {
            auto node = buffer_list_.front();
            auto ret = ::sendto(fd_, node->buf, node->len, 0, node->addr, node->addr_len);
            if (ret > 0) {
                buffer_list_.pop_front();
            } else if (ret < 0) {
                int err = errno;
                if (err != EINTR && err != EAGAIN && err != EWOULDBLOCK) {
                    NETLOG_ERROR << "udp socket sendto error, fd: " << fd_ << ", error: " << strerror(err);
                    onClose();
                    return;
                }
                break;
            }

            if (buffer_list_.empty()) {
                if (write_complete_cb_) {
                    write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
                }
                break;
            }
        }
    } else {
        enableWriting(false);
        if (write_complete_cb_) {
            write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        }
    }
}

void UdpSocket::onClose() {
    if (!closed_) {
        closed_ = true;
        if (close_cb_) {
            close_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
        }

        Event::close();
    }
}

void UdpSocket::onError(const std::string& msg) {
    NETLOG_ERROR << "udp socket error, fd: " << fd_ << ", peer: " << peerAddr_.toIpPort() << ", error: " << msg;
    onClose();
}

void UdpSocket::setTimeout(uint32_t timeout, const UdpSocketTimeoutCallback& cb) {
    auto udp_socket = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->runAfter(timeout, [udp_socket, cb]() {
        cb(udp_socket);
    });
}

void UdpSocket::setTimeout(uint32_t timeout, UdpSocketTimeoutCallback&& cb) {
    auto udp_socket = std::dynamic_pointer_cast<UdpSocket>(shared_from_this());
    loop_->runAfter(timeout, [udp_socket, cb]() {
        cb(udp_socket);
    });
}

void UdpSocket::enableCheckIdleTime(int32_t idle_time) {
    max_idle_time_ = idle_time;
    auto entry = std::make_shared<UdpTimeoutEntry>(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
    timeout_entry_ = entry;
    loop_->insertEntry(idle_time, entry);
}

void UdpSocket::extendLife() {
    auto entry = timeout_entry_.lock();
    if (entry) {
        loop_->insertEntry(max_idle_time_, entry);
    }
}

void UdpSocket::send(const void* buf, size_t len, struct sockaddr* addr, socklen_t addr_len) {
    loop_->runInLoop([this, buf, len, addr, addr_len]() {
        sendInLoop(buf, len, addr, addr_len);
    });
}

void UdpSocket::send(const std::list<UdpBufferNodePtr>& list, struct sockaddr* addr, socklen_t addr_len) {
    loop_->runInLoop([this, list, addr, addr_len]() {
        sendInLoop(list, addr, addr_len);
    });
}

void UdpSocket::sendInLoop(const void* buf, size_t len, struct sockaddr* addr, socklen_t addr_len) {
    if (buffer_list_.empty()) {
       int ret = ::sendto(fd_, buf, len, 0, addr, addr_len);
       if (ret > 0) {
            if (write_complete_cb_) {
                write_complete_cb_(std::dynamic_pointer_cast<UdpSocket>(shared_from_this()));
            }
            return;
       }
    }

    auto node = std::make_shared<UdpBufferNode>(buf, len, addr, addr_len);
    buffer_list_.emplace_back(node);
    enableWriting(true);
}

void UdpSocket::sendInLoop(const std::list<UdpBufferNodePtr>& list, struct sockaddr* addr, socklen_t addr_len) {
    for (auto& node : list) {
        buffer_list_.emplace_back(node);
    }
    if (!buffer_list_.empty()) {
        enableWriting(true);
    }
}

void UdpSocket::forceClose() {
    loop_->runInLoop([this]() {
        onClose();
    });
}
