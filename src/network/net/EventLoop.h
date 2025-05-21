#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <memory>

#include "Event.h"

namespace tmms {
    namespace network {

        using EventPtr = std::shared_ptr<Event>;

        class EventLoop {
        public:
            EventLoop();
            ~EventLoop();

            void loop();
            void quit();

            void addEvent(const EventPtr& event);
            void removeEvent(const EventPtr& event);
            void enableEventReading(const EventPtr& event, bool enable);
            void enableEventWriting(const EventPtr& event, bool enable);

        private:
            bool looping_ { false };
            int epoll_ { -1 };
            std::vector<struct epoll_event> epoll_events_;
            std::unordered_map<int, EventPtr> events_;
        };
    }
}


#endif
