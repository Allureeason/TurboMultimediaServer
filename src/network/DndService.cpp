#include "DnsService.h"
#include <functional>
#include <netdb.h>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

using namespace tmms::network;

namespace {
    static InetAddressPtr inet_address_null;
}

DnsService::DnsService() {
}

DnsService::~DnsService() {
}

void DnsService::addHost(const std::string& host) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = host_infos_.find(host);
    if (it == host_infos_.end()) {
        host_infos_[host] = std::vector<InetAddressPtr>();
    }
}

InetAddressPtr DnsService::getHostAddress(const std::string& host, int index) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = host_infos_.find(host);
    if (it == host_infos_.end()) {
        return inet_address_null;
    }
    
    if (it->second.size() > 0) {
        return it->second[index % it->second.size()];
    }
    return inet_address_null;
}

std::vector<InetAddressPtr> DnsService::getHostAddresses(const std::string& host) {
    std::lock_guard<std::mutex> lock(mtx_);
    auto it = host_infos_.find(host);
    if (it == host_infos_.end()) {
        return std::vector<InetAddressPtr>();
    }

    return it->second;
}

void DnsService::updateHostAddress(const std::string& host, std::vector<InetAddressPtr>& addrs) {
    std::lock_guard<std::mutex> lock(mtx_);
    host_infos_[host].swap(addrs);
}

std::unordered_map<std::string, std::vector<InetAddressPtr>> DnsService::getHostInfos() {
    std::lock_guard<std::mutex> lock(mtx_);
    return host_infos_;
}

void DnsService::setDnsServiceParams(int32_t interval, int32_t sleep, int32_t retry) {
    interval_ = interval;
    sleep_ = sleep;
    retry_ = retry;
}

void DnsService::start() {
    if (running_) {
        return;
    }
    running_ = true;
    thread_ = std::thread(std::bind(&DnsService::onWorker, this));
}

void DnsService::stop() {
    if (!running_) {
        return;
    }
    running_ = false;
    if (thread_.joinable()) {
        thread_.join();
    }
}

void DnsService::getHostInfo(const std::string& host, std::vector<InetAddressPtr>& addrs) {
    struct addrinfo* result = nullptr;
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;

    int ret = ::getaddrinfo(host.c_str(), nullptr, &hints, &result);
    if (ret != 0 || result == nullptr) {
        return;
    }

    for (struct addrinfo* p = result; p != nullptr; p = p->ai_next) {
        struct sockaddr_in6 addr_in;
        InetAddressPtr addr = std::make_shared<InetAddress>();
        if (p->ai_family == AF_INET6) {
            char ip[INET6_ADDRSTRLEN] = {0};
            struct sockaddr_in6* addr_in6 = reinterpret_cast<struct sockaddr_in6*>(p->ai_addr);
            ::inet_ntop(AF_INET6, &addr_in6->sin6_addr, ip, sizeof(ip));
            addr->setIP(ip);
            addr->setPort(ntohs(addr_in6->sin6_port));
        } else if (p->ai_family == AF_INET) {
            char ip[INET_ADDRSTRLEN] = {0};
            struct sockaddr_in* addr_in4 = reinterpret_cast<struct sockaddr_in*>(p->ai_addr);
            ::inet_ntop(AF_INET, &addr_in4->sin_addr, ip, sizeof(ip));
            addr->setIP(ip);
            addr->setPort(ntohs(addr_in4->sin_port));
            addr->setIsIPv6(false);
        }
        addrs.push_back(addr);
    }

    ::freeaddrinfo(result);
}

void DnsService::onWorker() {
    while (running_) {
        auto host_infos = getHostInfos();
        for (auto& host_info : host_infos) {
            for (int i = 0; i < retry_; i++) {
                std::vector<InetAddressPtr> addrs;
                getHostInfo(host_info.first, addrs);
                if (addrs.size() > 0) {
                    updateHostAddress(host_info.first, addrs);
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(sleep_));
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval_));
    }
}