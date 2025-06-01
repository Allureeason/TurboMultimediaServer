#ifndef UDP_SERVER_H
#define UDP_SERVER_H

#include "UdpSocket.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"

namespace tmms {
    namespace network {
        class UdpServer : public UdpSocket {
        public:
            UdpServer(EventLoop* loop, const InetAddress&  addr);
            virtual ~UdpServer();

            void start();
            void stop();
        private:
            void open();

        private:
            InetAddress addr_;
        };
    }
}

#endif 