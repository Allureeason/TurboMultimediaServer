#ifndef NETWORK_UDP_SOCKET_H
#define NETWORK_UDP_SOCKET_H

#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/Connection.h"
#include "base/MsgBuffer.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <list>

namespace tmms {
    namespace network {

        struct UdpBufferNode : public BufferNode {
            UdpBufferNode(const void* data, size_t len, struct sockaddr* paddr, socklen_t paddr_len)
            : BufferNode(data, len), addr(paddr), addr_len(paddr_len) {
            }

            struct sockaddr* addr { nullptr };
            socklen_t addr_len { 0 };
        };
        using UdpBufferNodePtr = std::shared_ptr<UdpBufferNode>;

        class UdpSocket;
        using UdpSocketPtr = std::shared_ptr<UdpSocket>;
        struct UdpTimeoutEntry;
        using UdpTimeoutEntryPtr = std::shared_ptr<UdpTimeoutEntry>;

        using UdpSocketMessageCallback = std::function<void(const InetAddress& addr, MsgBuffer& msg)>;
        using UdpSocketTimeoutCallback = std::function<void(const UdpSocketPtr& udp_socket)>;
        using UdpSocketWriteCompleteCallback = std::function<void(const UdpSocketPtr& udp_socket)>;
        using UdpSocketCloseCallback = std::function<void(const UdpSocketPtr& udp_socket)>;



        class UdpSocket : public Connection {
        public:
            UdpSocket(EventLoop* loop, int sockfd,const InetAddress& local_addr, const InetAddress& peer_addr);
            virtual ~UdpSocket();

            void onTimeout();

            void setMessageCallback(const UdpSocketMessageCallback& cb);
            void setMessageCallback(UdpSocketMessageCallback&& cb);
            void setWriteCompleteCallback(const UdpSocketWriteCompleteCallback& cb);
            void setWriteCompleteCallback(UdpSocketWriteCompleteCallback&& cb);
            void setCloseCallback(const UdpSocketCloseCallback& cb);
            void setCloseCallback(UdpSocketCloseCallback&& cb);

            void enableCheckIdleTime(int32_t idle_time);
            void setTimeout(uint32_t timeout, const UdpSocketTimeoutCallback& cb);
            void setTimeout(uint32_t timeout, UdpSocketTimeoutCallback&& cb);

            void onRead() override;
            void onWrite() override;
            void onClose() override;
            void onError(const std::string& msg) override;
            void forceClose() override;

            void send(const void* buf, size_t len, struct sockaddr* addr, socklen_t addr_len);
            void send(const std::list<UdpBufferNodePtr>& list, struct sockaddr* addr, socklen_t addr_len);

        private:
            void extendLife();
            void sendInLoop(const void* buf, size_t len, struct sockaddr* addr, socklen_t addr_len);
            void sendInLoop(const std::list<UdpBufferNodePtr>& list, struct sockaddr* addr, socklen_t addr_len);

        private:
            std::list<UdpBufferNodePtr> buffer_list_;
            bool closed_ { false };
            int32_t max_idle_time_ { 30 };
            int32_t message_buffer_size_ { 65535 };
            MsgBuffer message_buffer_;
            std::weak_ptr<UdpTimeoutEntry> timeout_entry_;

            UdpSocketMessageCallback message_cb_;
            UdpSocketWriteCompleteCallback write_complete_cb_;
            UdpSocketCloseCallback close_cb_;

        };

            struct UdpTimeoutEntry {
            UdpTimeoutEntry(UdpSocketPtr p) : udp_socket(p) {}
            ~UdpTimeoutEntry() {
                auto sock = udp_socket.lock();
                if (sock) {
                    sock->onTimeout();
                }
            }
            std::weak_ptr<UdpSocket> udp_socket;
        };
    }
}

#endif