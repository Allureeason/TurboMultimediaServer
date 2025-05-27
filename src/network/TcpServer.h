#ifndef NETWORK_TCP_SERVER_H
#define NETWORK_TCP_SERVER_H

#include "network/net/TcpConnection.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/Acceptor.h"
#include <unordered_set>

namespace tmms {
    namespace network {

        using NewConnectionCallback = std::function<void(const TcpConnectionPtr& conn)>;
        using DestroyConnectionCallback = std::function<void(const TcpConnectionPtr& conn)>;

        class TcpServer {
            public:
                TcpServer(EventLoop* loop, const InetAddress& addr);
                virtual ~TcpServer();
                
                void start();
                void stop();

                void setNewConnectionCallback(const NewConnectionCallback& cb);
                void setNewConnectionCallback(NewConnectionCallback&& cb);
                void setMessageCallback(const MessageCallback& cb);
                void setMessageCallback(MessageCallback&& cb);
                void setWriteCompleteCallback(const WriteCompleteCallback& cb);
                void setWriteCompleteCallback(WriteCompleteCallback&& cb);
                void setActiveCallback(const ActiveCallback& cb);
                void setActiveCallback(ActiveCallback&& cb);
                void setDestroyConnectionCallback(const DestroyConnectionCallback& cb);
                void setDestroyConnectionCallback(DestroyConnectionCallback&& cb);

                void onAccept(int sockfd, const InetAddress& peerAddr);
                void onConnectionClose(const TcpConnectionPtr& conn);

                void dumpConnections() const;

            private:
                EventLoop* loop_; // 事件循环
                InetAddress addr_;
                std::shared_ptr<Acceptor> acceptor_; // 连接接收器
                std::unordered_set<TcpConnectionPtr> connections_; // 保存tcp连接

                NewConnectionCallback new_connection_cb_; // 新连接回调
                MessageCallback message_cb_; // 消息回调
                WriteCompleteCallback write_complete_cb_; // 写完成回调
                ActiveCallback active_cb_; // 活跃回调
                DestroyConnectionCallback destroy_connection_cb_; // 销毁连接回调

        };
    }
}

#endif