#ifndef EVENT_LOOP_THREAD_H
#define EVENT_LOOP_THREAD_H

#include "EventLoop.h"
#include <thread>
#include <mutex>
#include <future>
#include <condition_variable>

#include "base/NonCopyable.h"

namespace tmms {
    namespace network {

        class EventLoopThread : public base::NonCopyable {
            public:
                EventLoopThread();
                ~EventLoopThread();

                void run();
                EventLoop* getLoop() const;
                
            private:
                void startLoop();

            private:
                bool running_ { false };
                EventLoop* loop_ { nullptr };
                std::thread thread_;
                std::mutex mtx_;
                std::condition_variable cv_;
                std::promise<int> promise_;
                std::once_flag once_;
        };
    }
}
#endif