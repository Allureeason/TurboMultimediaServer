#ifndef NETWORK_NET_TCPCONNECTION_H
#define NETWORK_NET_TCPCONNECTION_H

#include "Connection.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include "network/base/MsgBuffer.h"

#include <list>
#include <memory>
#include <sys/uio.h>

namespace tmms {
    namespace network {

        struct BufferNode {
            void * buf = nullptr;
            size_t len = 0;
        };
        using BufferNodePtr = std::shared_ptr<BufferNode>;

        class TcpConnection;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using CloseConnectionCallback = std::function<void(TcpConnectionPtr)>;
        using MessageCallback = std::function<void(TcpConnectionPtr, MsgBuffer&)>;
        using WriteCompleteCallback = std::function<void(TcpConnectionPtr)>;
        
        class TcpConnection : public Connection {
        public:
            TcpConnection(EventLoop* loop, int fd, const InetAddress& localAddr, const InetAddress& peerAddr);
            virtual ~TcpConnection();

            void setCloseCallback(const CloseConnectionCallback& cb);
            void setCloseCallback(CloseConnectionCallback&& cb);
            void onClose() override;
            void forceClose() override;

            void setRecvMessageCallback(const MessageCallback& cb);
            void setRecvMessageCallback(MessageCallback&& cb);
            void onRead() override;

            void onError(const std::string& msg) override;

            void setWriteCompleteCallback(const WriteCompleteCallback& cb);
            void setWriteCompleteCallback(WriteCompleteCallback&& cb);
            void send(const void * buf, size_t len);
            void send(const std::list<BufferNodePtr>& vec);
            void onWrite() override;

        private:
            void sendInLoop(const void * buf, size_t len);
            void sendInLoop(const std::list<BufferNodePtr>& vec);

        private:
            CloseConnectionCallback close_cb_;
            MessageCallback message_cb_;
            MsgBuffer message_buffer_;
            bool closed_ { true };

            std::vector<struct iovec> io_vec_list_;
            WriteCompleteCallback write_complete_cb_;
        };
    }
}

#endif