#ifndef __UDP_CLIENT_H__
#define __UDP_CLIENT_H__

#include "UdpSocket.h"

namespace tmms {
    namespace network {

        using UdpConnectionCallback = std::function<void(const UdpSocketPtr&, bool)>;


        class UdpClient : public UdpSocket {
        public:
            UdpClient(EventLoop* loop, const InetAddress& server_addr);
            virtual ~UdpClient();

            void connect();
            void setConnectionCallback(const UdpConnectionCallback& cb);
            void setConnectionCallback(UdpConnectionCallback&& cb);
            void send(const void* buf, size_t len);
            void send(const std::list<UdpBufferNodePtr>& list);
            void onClose() override;
            
        private:
            void connnectInLoop();

        private:
            bool connected_ { false };
            InetAddress server_addr_;
            UdpConnectionCallback connection_cb_;
            struct sockaddr_in6 sock_addr_;
            socklen_t sock_addr_len_ {sizeof(struct sockaddr_in6)};
        };
    }
}

#endif