#ifndef EVENT_LOOP_H
#define EVENT_LOOP_H

#include <sys/epoll.h>
#include <vector>
#include <unordered_map>
#include <memory>
#include <functional>
#include <mutex>
#include <queue>

#include "Event.h"
#include "PipeEvent.h"
#include "TimingWheel.h"

namespace tmms {
    namespace network {

        using EventPtr = std::shared_ptr<Event>;
        using Func = std::function<void()>;
        using PipeEventPtr = std::shared_ptr<PipeEvent>;

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

            void assertInLoopThread();
            bool isInLoopThread();

            void runInLoop(const Func& f);
            void runInLoop(Func&& f);

            void runAfter(int delay, const Func& f);
            void runAfter(int delay, Func&& f);
            void runEvery(int interval, const Func& f);
            void runEvery(int interval, Func&& f);
            void insertEntry(int delay, EntryPtr entry);

        private:
            void runFunctions();
            void weakup();

            bool looping_ { false };
            int epoll_ { -1 };
            std::vector<struct epoll_event> epoll_events_;
            std::unordered_map<int, EventPtr> events_;
            std::queue<Func> functions_;
            std::mutex func_mtx_;
            PipeEventPtr pipe_ { nullptr };

            TimingWheel timing_wheel_;
        };
    }
}


#endif
