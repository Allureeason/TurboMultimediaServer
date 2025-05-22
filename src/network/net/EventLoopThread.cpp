#include "EventLoopThread.h"

#include "network/base/Network.h"


using namespace tmms::network;

EventLoopThread::EventLoopThread() {
    thread_ = std::thread(&EventLoopThread::startLoop, this);
}

EventLoopThread::~EventLoopThread() {
    run();

    if (loop_) {
        loop_->quit();
    }

    if (thread_.joinable()) {
        thread_.join();
    }

}

void EventLoopThread::run() {
    std::call_once(once_, [this]() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            running_ = true;
        }
            cv_.notify_all();

        auto f = promise_.get_future();
        f.get();
    });
}

EventLoop* EventLoopThread::getLoop() const {
    return loop_;
}

void EventLoopThread::startLoop() {
    EventLoop loop;
    // 等待启动
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return running_; });
    }

    loop_ = &loop;
    promise_.set_value(1);

    NETLOG_INFO << "start EventLoop loop in thread ";

    // 启动事件循环
    loop.loop();
    
    running_ = false;
    loop_ = nullptr;
}

std::thread& EventLoopThread::getThread() {
    return thread_;
}