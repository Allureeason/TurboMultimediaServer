#ifndef INET_ADDRESS_H
#define INET_ADDRESS_H

#include <string>
#include <cstdint>


namespace tmms {
    namespace network {
        class InetAddress {
        public:
            InetAddress(const std::string& ip, uint16_t port, bool is_ipv6 = false);
            InetAddress(const std::string& host, bool is_ipv6 = false);
            InetAddress() = default;

            std::string getIP() const;
            uint16_t getPort() const;
            std::string toIpPort() const;

            uint32_t IPV4(const char* ip) const;
            uint32_t IPV4() const;
            void getSockAddress(struct sockaddr* addr) const;


            bool isIPv6() const;
            bool isIPv4() const;
            bool isLoopbackIP() const;
            bool isWanIP() const;
            bool isLanIP() const;


        private:
            std::string ip_;
            std::string port_;
            bool is_ipv6_ { false };
        };
    }
}


#endif