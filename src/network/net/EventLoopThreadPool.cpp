#include "EventLoopThreadPool.h"

using namespace tmms::network;

namespace {
    // 线程绑定cpu
    void bindCpu(std::thread& thread, int n) {
        cpu_set_t mask;
        CPU_ZERO(&mask);
        CPU_SET(n, &mask);

        pthread_setaffinity_np(thread.native_handle(), sizeof(mask), &mask);
    }
}

EventLoopThreadPool::EventLoopThreadPool(int num, int start, int cpus) {
    if (num <= 0) {
        num = 1;
    }

    if (start < 0) {
        start = 0;
    }

    for (int i = 0; i < num; i++) {
        auto elThread = std::make_shared<EventLoopThread>();
        
        if (cpus > 0) {
            int n = (start + i) % cpus;
            bindCpu(elThread->getThread(), n);
        }

        threads_.emplace_back(elThread);
    }
}

EventLoopThreadPool::~EventLoopThreadPool() {

}

std::vector<EventLoop*> EventLoopThreadPool::getEventLoops() {
    std::vector<EventLoop*> res;
    for (auto& t : threads_) {
        res.push_back(t->getLoop());
    }
    return res;
}

EventLoop* EventLoopThreadPool::getNextLoop() {
    int n = loop_index_++ % threads_.size();
    return threads_[n]->getLoop();
}

int EventLoopThreadPool::size() const {
    return threads_.size();
}

void EventLoopThreadPool::start() {
    for (auto& t : threads_) {
        t->run();
    }
}