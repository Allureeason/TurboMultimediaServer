#ifndef SOCKET_OPT_H
#define SOCKET_OPT_H

#include "InetAddress.h"
#include <memory>

namespace tmms {
    namespace network {

        using InetAddressPtr = std::shared_ptr<InetAddress>;

        class SocketOpt {
        public:
            SocketOpt(int sock, bool is_ipv6 = false);
            ~SocketOpt() = default;

            static int createNonblockingTcp(int family);
            static int createNonblockingUdp(int family);
            static std::string dumpSockAddr(const struct sockaddr* addr);

            InetAddressPtr getPeerAddr();
            InetAddressPtr getLocalAddr();
            int getSock() const;
            bool isIPV6() const;
            bool isIPV4() const;

            int bind(const InetAddress& addr);
            int listen();
            int accept(InetAddress* peer_addr);
            int connect(const InetAddress& addr);

            void setReuseAddr(bool on);
            void setReusePort(bool on);
            void setKeepAlive(bool on);
            void setTcpNoDelay(bool on);
            void setNonBlocking(bool on);
        

        private:
            int sock_ { -1 };
            bool is_ipv6_ { false };
        };

    }
}

#endif // SOCKET_OPT_H