#include "network/net/EventLoop.h"
#include "network/net/EventLoopThread.h"
#include "network/base/Network.h"
#include "network/net/PipeEvent.h"
#include "network/net/EventLoopThreadPool.h"

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

void testEventLoopThreadPool() {
    EventLoopThreadPool pool(4, 0, 4);
    pool.start();

    // for (int i = 0; i < pool.size(); i++) {
    //     auto loop = pool.getNextLoop();
    //     NETLOG_INFO << "loop: " << loop;
    // }
    
    // int cnt = 0;
    // while (true) {
    //     auto loop = pool.getNextLoop();
    //     loop->runInLoop([&cnt]() {
    //         NETLOG_INFO << "cnt: " << ++cnt;
    //     });
    //     std::this_thread::sleep_for(std::chrono::seconds(1));
    // }

    auto loop = pool.getNextLoop();
    loop->runAfter(1, [](){
        NETLOG_INFO << "1s run after timer";
    });


    loop->runAfter(10, [](){
        NETLOG_INFO << "10s run after timer";
    });

    loop->runEvery(5, [](){
        NETLOG_INFO << "5s run every timer";
    });

    loop->runEvery(10, [](){
        NETLOG_INFO << "10s run every timer";
    });


    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}


int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);

    // testEventLoop();

    testEventLoopThreadPool();

    return 0;
}