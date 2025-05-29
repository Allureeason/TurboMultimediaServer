#include "network/DnsService.h"
#include "base/LogStream.h"
#include "network/base/Network.h"

using namespace tmms::network;

int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);


    std::vector<InetAddressPtr> addrs;
    DnsService::getHostInfo("www.baidu.com", addrs);

    for (auto& addr : addrs) {
        NETLOG_INFO << "addr: " << addr->toIpPort();
    }

    DnsService dns_service;
    dns_service.addHost("www.baidu.com");
    dns_service.start();

    std::this_thread::sleep_for(std::chrono::seconds(3));

    // 通过dnsservice获取地址
    NETLOG_INFO << "get host address";
    auto addr = dns_service.getHostAddress("www.baidu.com");
    NETLOG_INFO << "addr: " << addr->toIpPort();

    // 通过dnsservice获取地址列表
    NETLOG_INFO << "get host addresses";
    auto list = dns_service.getHostAddresses("www.baidu.com");
    for (auto& addr : list) {
        NETLOG_INFO << "addr: " << addr->toIpPort();
    }

    // 获取全部host
    NETLOG_INFO << "get host infos";
    auto host_infos = dns_service.getHostInfos();
    for (auto& host_info : host_infos) {
        NETLOG_INFO << "host: " << host_info.first;
        for (auto& addr : host_info.second) {
            NETLOG_INFO << "addr: " << addr->toIpPort();
        }

        NETLOG_INFO << "--------------------------------";
    }

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    dns_service.stop();

    return 0;
}