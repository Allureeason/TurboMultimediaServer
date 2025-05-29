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

        class TcpConnection;
        using TcpConnectionPtr = std::shared_ptr<TcpConnection>;
        using CloseConnectionCallback = std::function<void(TcpConnectionPtr)>;
        using MessageCallback = std::function<void(TcpConnectionPtr, MsgBuffer&)>;
        using WriteCompleteCallback = std::function<void(TcpConnectionPtr)>;
        using TimeoutCallback = std::function<void(TcpConnectionPtr)>;

        struct TimeoutEntry;
        
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

            void setTimeout(uint32_t timeout, const TimeoutCallback& cb);
            void setTimeout(uint32_t timeout, TimeoutCallback&& cb);
            void onTimeout();
            void enableCheckIdleTime(uint32_t timeout);

        private:
            void sendInLoop(const void * buf, size_t len);
            void sendInLoop(const std::list<BufferNodePtr>& vec);
            void extendLife();

        private:
            CloseConnectionCallback close_cb_;
            MessageCallback message_cb_;
            MsgBuffer message_buffer_;
            bool closed_ { false };

            std::vector<struct iovec> io_vec_list_;
            WriteCompleteCallback write_complete_cb_;

            std::weak_ptr<TimeoutEntry> timeout_entry_;
            uint32_t max_idle_ms_ { 30 };
        };

        struct TimeoutEntry {
            TimeoutEntry(TcpConnectionPtr conn) : conn(conn) {}
            ~TimeoutEntry() {
                auto c = conn.lock();
                if (c) {
                    c->onTimeout();
                }
            }
            std::weak_ptr<TcpConnection> conn;
        };
    }
}

#endif