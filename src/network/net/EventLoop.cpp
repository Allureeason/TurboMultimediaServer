#include "EventLoop.h"
#include "network/base/Network.h"
#include "base/TTime.h"

#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace tmms::network;

static thread_local EventLoop* t_event_loop = nullptr;

EventLoop::EventLoop()
    : epoll_(epoll_create(1024))
    , epoll_events_(1024) {

    if (epoll_ < 0) {
        NETLOG_ERROR << "EventLoop::EventLoop(): "
                     << "epoll_create failed: " << strerror(errno);
        exit(-1);
    }

    if (t_event_loop) {
        NETLOG_ERROR << "Another EventLoop " << t_event_loop
                     << " exists in this thread " << pthread_self();
        exit(-1);
    }

    NETLOG_INFO << "EventLoop::EventLoop(): "
                << " loop: " << this
                << " epoll: " << epoll_
                << " events: " << epoll_events_.size();
    t_event_loop = this;
}

EventLoop::~EventLoop() {
    quit();
}

void EventLoop::loop() {
    looping_ = true;
    int timeout = 1000;

    while (looping_) {
        memset(&epoll_events_[0], 0, epoll_events_.size() * sizeof(struct epoll_event));


        int ret = epoll_wait(epoll_,
                        &epoll_events_[0],
                        static_cast<int>(epoll_events_.size()),
                        timeout);

        NETLOG_TRACE << "event return count: " << ret;
        for (const auto& it : events_) {
            NETLOG_TRACE << "fd: " << it.first << " event use count: " << it.second.use_count();
        }

        if (ret >= 0) {
            for (int i = 0; i < ret; ++i) {
                NETLOG_DEBUG << "epoll_wait event: " << i << " fd: " << epoll_events_[i].data.fd << " events: " << epoll_events_[i].events;
                struct epoll_event& e = epoll_events_[i];
                int fd = e.data.fd;

                auto it = events_.find(fd);
                if (it == events_.end()) {
                    NETLOG_ERROR << "EventLoop::loop(): "
                                 << "fd " << fd << " not found";
                    continue;
                }

                EventPtr& event = it->second;

                if (e.events & EPOLLERR) {
                    // fd error
                    int err = 0;
                    socklen_t len = sizeof(err);
                    getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &len);
                    event->onError(strerror(err));

                } else if ((e.events & EPOLLHUP) && !(e.events & EPOLLIN)) {
                    // fd hang up
                    event->onClose();
                } else if (e.events & (EPOLLIN | EPOLLPRI)) {
                    // fd readable
                    event->onRead();
                } else if (e.events & EPOLLOUT) {
                    // fd writable
                    event->onWrite();
                }
            }

            // resize epoll_events_ if needed
            if (ret == static_cast<int>(epoll_events_.size())) {
                epoll_events_.resize(epoll_events_.size() * 2);
            }

            // 执行任务队列
            runFunctions();

            // 检查并执行定时任务
            auto now = tmms::base::TTime::nowMs();
            timing_wheel_.onTimer(now);
            
        } else if (ret < 0) {
            // error
            NETLOG_ERROR << "epoll_wait error: " << strerror(errno);
        }
        
    }
}

void EventLoop::quit() {
    looping_ = false;
    ::close(epoll_);
}

void EventLoop::addEvent(const EventPtr& event) {
    int fd = event->getFd();
    auto it = events_.find(fd);
    if (it != events_.end()) {
        NETLOG_ERROR << "EventLoop::addEvent(): "
                     << "fd " << fd << " already exists";
        return;
    }

    event->setEvents(kEventRead);
    events_[fd] = event;

    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.fd = fd;
    e.events = event->getEvents();
    epoll_ctl(epoll_, EPOLL_CTL_ADD, fd, &e);

    NETLOG_DEBUG << "fd " << fd << " added to epoll events: " << e.events;
}

void EventLoop::removeEvent(const EventPtr& event) {
    int fd = event->getFd();
    auto it = events_.find(fd);
    if (it == events_.end()) {
        return;
    }

    events_.erase(it);

    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.fd = fd;
    epoll_ctl(epoll_, EPOLL_CTL_DEL, fd, &e);
    NETLOG_DEBUG << "fd " << fd << " removed from epoll";
}

void EventLoop::enableEventReading(const EventPtr& event, bool enable) {
    int fd = event->getFd();
    auto it = events_.find(fd);
    if (it == events_.end()) {
        NETLOG_ERROR << "EventLoop::enableEventReading(): "
                     << "fd " << fd << " not found";
        return;
    }

    if (enable) {
        event->setEvents(event->getEvents() | kEventRead);
    } else {
        event->setEvents(event->getEvents() & ~kEventRead);
    }

    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.fd = fd;
    e.events = event->getEvents();
    int ret = epoll_ctl(epoll_, EPOLL_CTL_MOD, fd, &e);
    if (ret < 0) {
        NETLOG_ERROR << "EventLoop::enableEventReading(): "
                     << "fd " << fd << " enable reading failed: " << strerror(errno);
    } else {
        NETLOG_DEBUG << "fd " << fd << " enabled reading: " << e.events;
    }
}

void EventLoop::enableEventWriting(const EventPtr& event, bool enable) {
    int fd = event->getFd();
    auto it = events_.find(fd);
    if (it == events_.end()) {
        NETLOG_ERROR << "EventLoop::enableEventWriting(): "
                     << "fd " << fd << " not found";
        return;
    }

    if (enable) {
        event->setEvents(event->getEvents() | kEventWrite);
    } else {
        event->setEvents(event->getEvents() & ~kEventWrite);
    }

    struct epoll_event e;
    memset(&e, 0, sizeof(e));
    e.data.fd = fd;
    e.events = event->getEvents();
    int ret = epoll_ctl(epoll_, EPOLL_CTL_MOD, fd, &e);
    if (ret < 0) {
        NETLOG_ERROR << "EventLoop::enableEventWriting(): "
                     << "fd " << fd << " enable writing failed: " << strerror(errno);
    } else {
        NETLOG_DEBUG << "fd " << fd << " enabled writing: " << e.events;
    }
}

void EventLoop::runInLoop(const Func& f) {
    if (isInLoopThread()) {
        f();
    } else {
        std::lock_guard<std::mutex> lk(func_mtx_);
        functions_.push(f);
        weakup();
    }

}

void EventLoop::runInLoop(Func&& f) {
    if (isInLoopThread()) {
        f();
    } else {
        std::lock_guard<std::mutex> lk(func_mtx_);
        functions_.push(std::move(f));
        weakup();
    }
}

bool EventLoop::isInLoopThread() {
    return t_event_loop == this;
}

void EventLoop::assertInLoopThread() {
    if (!isInLoopThread()) {
        NETLOG_ERROR << "It is forbidden to run loop on other thread.";
        exit(-1);
    }
}

void EventLoop::runFunctions() {
    std::lock_guard<std::mutex> lk(func_mtx_);
    while (!functions_.empty()) {
        auto f = functions_.front();
        f();
        functions_.pop();
    }
}

void EventLoop::weakup() {
    if (!pipe_) {
        pipe_ = std::make_shared<PipeEvent>(this);
        addEvent(pipe_);
    }

    // 向写管道fd写入数据，触发读fd的可读时间，从而唤醒epoll
    int64_t tmp = 1;
    pipe_->write((const char*)&tmp, sizeof(tmp));
}

void EventLoop::runAfter(int delay, const Func& f) {
    if (isInLoopThread()) {
        timing_wheel_.runAfter(delay, f);
    } else {
        runInLoop([this, delay, f]() {
            timing_wheel_.runAfter(delay, f);
        });
    }
}

void EventLoop::runAfter(int delay, Func&& f)  {
    if (isInLoopThread()) {
        timing_wheel_.runAfter(delay, f);
    } else {
        runInLoop([this, delay, f]() {
            timing_wheel_.runAfter(delay, f);
        });
    }
}

void EventLoop::runEvery(int interval, const Func& f) {
    if (isInLoopThread()) {
        timing_wheel_.runAfter(interval, f);
    } else {
        runInLoop([this, interval, f]() {
            timing_wheel_.runEvery(interval, f);
        });
    }
}

void EventLoop::runEvery(int interval, Func&& f) {
    if (isInLoopThread()) {
        timing_wheel_.runAfter(interval, f);
    } else {
        runInLoop([this, interval, f]() {
            timing_wheel_.runEvery(interval, f);
        });
    }
}

void EventLoop::insertEntry(int delay, EntryPtr entry) {
    if (isInLoopThread()) {
        timing_wheel_.insertEntry(delay, entry);
    } else {
        runInLoop([this, delay, entry]() {
            timing_wheel_.insertEntry(delay, entry);
        });
    }
}