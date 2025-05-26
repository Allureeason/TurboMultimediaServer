#ifndef EVENT_H
#define EVENT_H

#include <string>
#include <memory>
#include <sys/epoll.h>

namespace tmms {
    namespace network {

        const int kEventRead = (EPOLLIN | EPOLLPRI);
        const int kEventWrite = (EPOLLOUT);

        class EventLoop;
        class Event : public std::enable_shared_from_this<Event> {
            public:
            Event(EventLoop* loop);
                Event(EventLoop* loop, int fd);
                ~Event();

                virtual void onRead();
                virtual void onWrite();
                virtual void onError(const std::string& msg);
                virtual void onClose();
                
                void enableReading(bool enable);
                void enableWriting(bool enable);

                void setEvents(int events);
                int getEvents() const;

                int getFd() const;
                void close();
                
            protected:
                EventLoop* loop_ { nullptr };
                int fd_ {-1};
                int events_ {0};
        };
    }
}

#endif
