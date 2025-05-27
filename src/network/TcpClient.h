#ifndef NETWORK_TCP_CLIENT_H
#define NETWORK_TCP_CLIENT_H

#include "network/net/TcpConnection.h"

namespace tmms {
    namespace network {

        enum {
            kTcpConnStatusInit = 0,
            kTcpConnStatusConnecting = 1,
            kTcpConnStatusConnected = 2,
            kTcpConnStatusDisconnected = 3,
            kTcpConnStatusMax
        };

        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using ConnectionCallback = std::function<void(const TcpConnectionPtr& conn, bool isConnected)>;

        class TcpClient : public TcpConnection {
            public:
                TcpClient(EventLoop* loop, const InetAddress& serverAddr);
                virtual ~TcpClient();

                void setConnectionCallback(const ConnectionCallback& cb);
                void setConnectionCallback(ConnectionCallback&& cb);
                void connect();
                void connectInLoop();
                void updateConnectionStatus();

                void onRead() override;
                void onWrite() override;
                void onClose() override;

                void send(std::list<BufferNodePtr>& vec);
                void send(const void* buf, size_t len);

            private:
                bool checkError();

            private:
                InetAddress server_addr_;
                int32_t status_ { kTcpConnStatusInit };
                ConnectionCallback connection_cb_;
        };
    }
}

#endif