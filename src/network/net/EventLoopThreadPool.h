#ifndef EVENTLOOPTHREADPOOL_H
#define EVENTLOOPTHREADPOOL_H

#include <vector>
#include <memory>
#include <functional>
#include "EventLoop.h"
#include "EventLoopThread.h"
#include "base/NonCopyable.h"

namespace tmms {
    namespace network {

        using EventLoopThreadPtr = std::shared_ptr<EventLoopThread>;

        class EventLoopThreadPool : public base::NonCopyable {
            public:
                EventLoopThreadPool(int num, int start, int cpus);
                ~EventLoopThreadPool();

                std::vector<EventLoop*> getEventLoops();
                EventLoop* getNextLoop();
                int size() const;
                void start();
            private:
                std::vector<EventLoopThreadPtr> threads_;
                int loop_index_ {0};
        };
    }
}

#endif
