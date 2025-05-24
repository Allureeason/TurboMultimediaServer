#include "InetAddress.h"
#include "network/base/Network.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <cstring>

using namespace tmms::network;


InetAddress::InetAddress(const std::string& ip, uint16_t port, bool is_ipv6)
    : ip_(ip), port_(std::to_string(port)), is_ipv6_(is_ipv6) {
}

InetAddress::InetAddress(const std::string& host, bool is_ipv6)
    : is_ipv6_(is_ipv6) {
    auto pos = host.find(':');
    if (pos == std::string::npos) {
        ip_ = host;
    } else {
        ip_ = host.substr(0, pos);
        port_ = host.substr(pos + 1);
    }
}

std::string InetAddress::getIP() const {
    return ip_;
}

uint16_t InetAddress::getPort() const {
    if (port_.empty()) {
        return 0;
    }
    return std::stoi(port_);
}

std::string InetAddress::toIpPort() const {
    return ip_ + ":" + port_;
}

uint32_t InetAddress::IPV4(const char* ip) const {
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(0);
    int ret = inet_pton(AF_INET, ip, &addr.sin_addr);
    if (!ret) {
        NETLOG_ERROR << "convert ip to ipv4 failed, ip: " << ip;
    }
    return ntohl(addr.sin_addr.s_addr);
}

uint32_t InetAddress::IPV4() const {
    return IPV4(ip_.c_str());
}

void InetAddress::getSockAddress(struct sockaddr* addr) const {
    if (is_ipv6_) {
        struct sockaddr_in6* addr6 = reinterpret_cast<sockaddr_in6*>(addr);
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(std::stoi(port_));
        inet_pton(AF_INET6, ip_.c_str(), &addr6->sin6_addr);
    } else {
        sockaddr_in* addr4 = reinterpret_cast<sockaddr_in*>(addr);
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(std::stoi(port_));
        inet_pton(AF_INET, ip_.c_str(), &addr4->sin_addr);
    }
}


bool InetAddress::isIPv6() const {
    return is_ipv6_;
}

bool InetAddress::isIPv4() const {
    return !is_ipv6_;
}

bool InetAddress::isLoopbackIP() const {
    return ip_ == "127.0.0.1" || ip_ == "::1";
}

bool InetAddress::isWanIP() const {
    uint32_t aStart = IPV4("10.0.0.0");
    uint32_t aEnd = IPV4("10.255.255.255");
    uint32_t bStart = IPV4("172.16.0.0");
    uint32_t bEnd = IPV4("172.31.255.255");
    uint32_t cStart = IPV4("192.168.0.0");
    uint32_t cEnd = IPV4("192.168.255.255");

    uint32_t ip = IPV4();

    bool isA = ip >= aStart && ip <= aEnd;
    bool isB = ip >= bStart && ip <= bEnd;
    bool isC = ip >= cStart && ip <= cEnd;

    return !isA && !isB && !isC&&ip!=INADDR_LOOPBACK;
    
}

bool InetAddress::isLanIP() const {
    uint32_t aStart = IPV4("10.0.0.0");
    uint32_t aEnd = IPV4("10.255.255.255");
    uint32_t bStart = IPV4("172.16.0.0");
    uint32_t bEnd = IPV4("172.31.255.255");
    uint32_t cStart = IPV4("192.168.0.0");
    uint32_t cEnd = IPV4("192.168.255.255");

    uint32_t ip = IPV4();

    bool isA = ip >= aStart && ip <= aEnd;
    bool isB = ip >= bStart && ip <= bEnd;
    bool isC = ip >= cStart && ip <= cEnd;

    return isA || isB || isC;
}

void InetAddress::setIP(const std::string& ip) {
    ip_ = ip;
}

void InetAddress::setPort(uint16_t port) {
    port_ = std::to_string(port);
}

void InetAddress::setIsIPv6(bool is_ipv6) {
    is_ipv6_ = is_ipv6;
}