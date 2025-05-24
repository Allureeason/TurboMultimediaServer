#include "SocketOpt.h"
#include "network/base/Network.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>

using namespace tmms::network;

SocketOpt::SocketOpt(int sock, bool is_ipv6)
    : sock_(sock), is_ipv6_(is_ipv6) {

}


int SocketOpt::createNonblockingTcp(int family) {
    int sock = ::socket(family, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP);
    if (sock < 0) {
        NETLOG_ERROR << "createNonblockingTcp error: " << strerror(errno);
    }
    return sock;
}

int SocketOpt::createNonblockingUdp(int family) {
    int sock = ::socket(family, SOCK_DGRAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_UDP);
    if (sock < 0) {
        NETLOG_ERROR << "createNonblockingUdp error: " << strerror(errno);
    }
    return sock;
}

int SocketOpt::bind(const InetAddress& addr) {
    if (addr.isIPv6()) {
        struct sockaddr_in6 addr_in;
        addr.getSockAddress((struct sockaddr*)&addr_in);
        return ::bind(sock_, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    } else {
        struct sockaddr_in addr_in;
        addr.getSockAddress((struct sockaddr*)&addr_in);
        return ::bind(sock_, (const struct sockaddr*)&addr_in, sizeof(addr_in));
    }
}

int SocketOpt::listen() {
    return ::listen(sock_, SOMAXCONN);
}

int SocketOpt::accept(InetAddress* peer_addr) {
    struct sockaddr_in6 addr_in;
    socklen_t addr_len = sizeof(addr_in);
    int fd = ::accept4(sock_, (struct sockaddr*)&addr_in, &addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
    if (fd != -1) {
        if (addr_in.sin6_family == AF_INET6) {
            char ip[INET6_ADDRSTRLEN] = {0};
            ::inet_ntop(AF_INET6, &addr_in.sin6_addr, ip, sizeof(ip));
            peer_addr->setIP(ip);
            peer_addr->setPort(ntohs(addr_in.sin6_port));
        } else {
            char ip[INET_ADDRSTRLEN] = {0};
            struct sockaddr_in* addr_in4 = reinterpret_cast<struct sockaddr_in*>(&addr_in);
            ::inet_ntop(AF_INET, &addr_in4->sin_addr, ip, sizeof(ip));
            peer_addr->setIP(ip);
            peer_addr->setPort(ntohs(addr_in4->sin_port));
            peer_addr->setIsIPv6(false);
        }
    }
    return fd;
}

int SocketOpt::connect(const InetAddress& addr) {
    struct sockaddr_in6 addr_in;
    addr.getSockAddress((struct sockaddr*)&addr_in);
    return ::connect(sock_, (struct sockaddr*)&addr_in, sizeof(addr_in));
}

InetAddressPtr SocketOpt::getPeerAddr() {
    struct sockaddr_in6 addr_in;
    socklen_t addr_len = sizeof(addr_in);
    ::getpeername(sock_, (struct sockaddr*)&addr_in, &addr_len);
    InetAddressPtr addr = std::make_shared<InetAddress>();
     if (addr_in.sin6_family == AF_INET6) {
        char ip[INET6_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET6, &addr_in.sin6_addr, ip, sizeof(ip));
        addr->setIP(ip);
        addr->setPort(ntohs(addr_in.sin6_port));
    } else {
        char ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in* addr_in4 = reinterpret_cast<struct sockaddr_in*>(&addr_in);
        ::inet_ntop(AF_INET, &addr_in4->sin_addr, ip, sizeof(ip));
        addr->setIP(ip);
        addr->setPort(ntohs(addr_in4->sin_port));
        addr->setIsIPv6(false);
    }
    return addr;
}

InetAddressPtr SocketOpt::getLocalAddr() {
    struct sockaddr_in6 addr_in;
    socklen_t addr_len = sizeof(addr_in);
    ::getsockname(sock_, (struct sockaddr*)&addr_in, &addr_len);
    InetAddressPtr addr = std::make_shared<InetAddress>();
     if (addr_in.sin6_family == AF_INET6) {
        char ip[INET6_ADDRSTRLEN] = {0};
        ::inet_ntop(AF_INET6, &addr_in.sin6_addr, ip, sizeof(ip));
        addr->setIP(ip);
        addr->setPort(ntohs(addr_in.sin6_port));
    } else {
        char ip[INET_ADDRSTRLEN] = {0};
        struct sockaddr_in* addr_in4 = reinterpret_cast<struct sockaddr_in*>(&addr_in);
        ::inet_ntop(AF_INET, &addr_in4->sin_addr, ip, sizeof(ip));
        addr->setIP(ip);
        addr->setPort(ntohs(addr_in4->sin_port));
        addr->setIsIPv6(false);
    }
    return addr;
}

int SocketOpt::getSock() const {
    return sock_;
}

bool SocketOpt::isIPV6() const {
    return is_ipv6_;
}

bool SocketOpt::isIPV4() const {
    return !is_ipv6_;
}

void SocketOpt::setReuseAddr(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
}

void SocketOpt::setReusePort(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

void SocketOpt::setKeepAlive(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_, SOL_SOCKET, SO_KEEPALIVE, &optval, sizeof(optval));
}

void SocketOpt::setTcpNoDelay(bool on) {
    int optval = on ? 1 : 0;
    ::setsockopt(sock_, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void SocketOpt::setNonBlocking(bool on) {
    int flags = ::fcntl(sock_, F_GETFL, 0);
    if (on) {
        flags |= O_NONBLOCK;
    } else {
        flags &= ~O_NONBLOCK;
    }
    ::fcntl(sock_, F_SETFL, flags);
}
