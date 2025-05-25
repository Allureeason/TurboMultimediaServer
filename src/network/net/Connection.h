#ifndef NETWORK_NET_CONNECTION_H
#define NETWORK_NET_CONNECTION_H

#include "EventLoop.h"
#include "Event.h"
#include "InetAddress.h"

#include <unordered_map>
#include <memory>
#include <atomic>

namespace tmms {
    namespace network {

        enum {
            kNormalContext = 0,
            kHttpContext = 1,
            kRtmpContext = 3,
            kRtpContext = 4,
            kFlvContext = 5,
            kUserContext = 6,

            kContextCount
        };

        class Connection;
        using ConnectionPtr = std::shared_ptr<Connection>;
        using ContextPtr = std::shared_ptr<void>;
        using ActiveCallback = std::function<void(ConnectionPtr)>;

        class Connection : public Event {
        public:
            Connection(EventLoop* loop, int fd, const InetAddress& localAddr, const InetAddress& peerAddr);
            virtual ~Connection() = default;

            void setPeerAddr(const InetAddress& addr);
            void setLocalAddr(const InetAddress& addr);
            const InetAddress& getPeerAddr() const;
            const InetAddress& getLocalAddr() const;

            void setContext(int type, const ContextPtr& ctx);
            void setContext(int type, ContextPtr&& ctx);

            template<typename T>
            std::shared_ptr<T> getContext(int type) {
                auto it = contexts_.find(type);
                if (it != contexts_.end()) {
                    return std::static_pointer_cast<T>(it->second);
                }
                return nullptr;
            }

            void removeContext(int type);
            void clearContexts();
            void setActiveCallback(const ActiveCallback& cb);
            void setActiveCallback(ActiveCallback&& cb);
            void active();
            void deactive();

            virtual void forceClose() = 0;

        private:
            std::unordered_map<int, ContextPtr> contexts_;
            ActiveCallback active_cb_;
            std::atomic<bool> active_ { false };

        protected:
            InetAddress localAddr_;
            InetAddress peerAddr_;
        };
    }
}

#endif