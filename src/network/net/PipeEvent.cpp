#include "PipeEvent.h"
#include "network/base/Network.h"

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <cstdint>
using namespace tmms::network;

PipeEvent::PipeEvent(EventLoop* loop)
    : Event(loop) {
    int pipe_fd[2] = {-1, -1};
    if (pipe(pipe_fd) == -1) {
        NETLOG_ERROR << "pipe error: " << strerror(errno);
        return;
    }
    fd_ = pipe_fd[0];
    write_fd_ = pipe_fd[1];
}

PipeEvent::~PipeEvent() {
    ::close(write_fd_);
}

void PipeEvent::onRead() {
    int64_t v = 1;    
    ssize_t n = ::read(fd_, &v, sizeof(v));
    if (n > 0) {
        NETLOG_INFO << "trigger onRead, read from pipe: " << v;
    }
}

void PipeEvent::write(const char* data, size_t len) {
    ::write(write_fd_, data, len);
}
