#ifndef NETWORK_NET_ACCEPTOR_H
#define NETWORK_NET_ACCEPTOR_H

#include "SocketOpt.h"
#include "InetAddress.h"
#include "EventLoop.h"
#include "Event.h"
#include <functional>

namespace tmms {
    namespace network {

        using AcceptCallback = std::function<void(int sock, const InetAddress& addr)>;

        class Acceptor : public Event {
        public:
            Acceptor(EventLoop* loop, const InetAddress& addr);
            ~Acceptor();

            void setAcceptCallback(const AcceptCallback& cb);
            void setAcceptCallback(AcceptCallback&& cb);

            void start();
            void stop();

            void onRead() override;
            void onError(const std::string& msg) override;
            void onClose() override;

        private:
            void open();

        private:
            InetAddress addr_;
            SocketOpt* sockopt_ { nullptr };
            AcceptCallback accept_callback_ { nullptr };
        };

    }
}

#endif