#include "network/net/InetAddress.h"
#include "network/base/Network.h"
#include <iostream>

using namespace tmms::network;

void testAddress(const std::string& addr) {
    InetAddress inetAddr(addr);
    std::cout << "addr: " << addr << std::endl;
    std::cout << "getIP: " << inetAddr.getIP() << std::endl;
    std::cout << "getPort: " << inetAddr.getPort() << std::endl;
    std::cout << "toIpPort: " << inetAddr.toIpPort() << std::endl;
    std::cout << "isIPv6: " << inetAddr.isIPv6() << std::endl;
    std::cout << "isIPv4: " << inetAddr.isIPv4() << std::endl;
    std::cout << "isLoopbackIP: " << inetAddr.isLoopbackIP() << std::endl;
    std::cout << "isWanIP: " << inetAddr.isWanIP() << std::endl;
    std::cout << "isLanIP: " << inetAddr.isLanIP() << std::endl;
}

void testInetAddress() {
    // 测试回环地址
    NETLOG_INFO << "测试回环地址:";
    testAddress("127.0.0.1");
    testAddress("127.0.0.1:8080");
    
    // 测试本地局域网地址
    NETLOG_INFO << "测试局域网地址:";
    testAddress("192.168.1.1");
    testAddress("10.0.0.1");
    testAddress("172.16.0.1:8080");
    
    // 测试公网地址
    NETLOG_INFO << "测试公网地址:";
    testAddress("8.8.8.8");
    testAddress("114.114.114.114:53");
    
    // 测试IPv6地址
    NETLOG_INFO << "测试IPv6地址:";
    InetAddress ipv6("::1", 8080, true);
    std::cout << "IPv6回环: " << ipv6.toIpPort() << std::endl;
    std::cout << "isIPv6: " << ipv6.isIPv6() << std::endl;
    std::cout << "isLoopbackIP: " << ipv6.isLoopbackIP() << std::endl;
    
    // 测试特殊情况
    NETLOG_INFO << "测试特殊情况:";
    InetAddress emptyAddr;
    std::cout << "空地址: " << emptyAddr.toIpPort() << std::endl;
    
    try {
        InetAddress badAddr("not.an.ip.address:abcd");
        std::cout << "错误地址解析: " << badAddr.toIpPort() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "捕获异常: " << e.what() << std::endl;
    }
    
    // 测试端口解析
    InetAddress portTest1("127.0.0.1:1234");
    InetAddress portTest2("127.0.0.1", (uint16_t)1234);
    std::cout << "端口解析测试: " 
              << (portTest1.getPort() == portTest2.getPort() ? "通过" : "失败")
              << std::endl;
}

int main(int argc, const char* argv[]) {
    (void)argc;
    (void)argv;

    tmms::base::g_logger = new tmms::base::Logger(nullptr, "TestInetAddress");
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);

    testInetAddress();

    return 0;
}