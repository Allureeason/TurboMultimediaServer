#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/base/Network.h"
#include "network/net/PipeEvent.h"

#include "base/LogStream.h"


using namespace tmms::network;

void testEventLoop() {
    EventLoopThread event_loop_thread;
    event_loop_thread.run();

    EventLoop* loop = event_loop_thread.getLoop();
    
    NETLOG_INFO << "main loop: " << loop;

    auto pipe_event = std::make_shared<PipeEvent>(loop);
    loop->addEvent(pipe_event);

    while (true) {
        // 往管道中写入数据
        int64_t v = 1;
        pipe_event->write((const char*)&v, sizeof(v));
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);

    testEventLoop();

    return 0;
}