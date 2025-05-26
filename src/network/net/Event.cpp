#include "Event.h"
#include "network/base/Network.h"
#include "EventLoop.h"
#include <unistd.h>

using namespace tmms::network;

Event::Event(EventLoop* loop)
    : loop_(loop), fd_(-1), events_(0) {
}

Event::Event(EventLoop* loop, int fd)
    : loop_(loop), fd_(fd), events_(0) {
}

Event::~Event() {
    close();
}

void Event::onRead() {
}

void Event::onWrite() {
}

void Event::onError(const std::string& msg) {
    NETLOG_ERROR << "Event::onError: " << msg;
}

void Event::onClose() {
}

void Event::enableReading(bool enable) {
    loop_->enableEventReading(shared_from_this(), enable);
}

void Event::enableWriting(bool enable) {
    loop_->enableEventWriting(shared_from_this(), enable);
}

void Event::setEvents(int events) {
    events_ = events;
}

int Event::getEvents() const {
    return events_;
}

int Event::getFd() const {
    return fd_;
}

void Event::close() {
    if (fd_ != -1) {
        ::close(fd_);
        fd_ = -1;
    }
}