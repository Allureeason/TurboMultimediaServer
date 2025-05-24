#include "network/net/Acceptor.h"
#include "network/base/Network.h"
#include "network/net/EventLoop.h"
#include "network/net/InetAddress.h"
#include "network/net/EventLoopThread.h"

using namespace tmms::network;

int main() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);

    auto thr =std::make_shared<EventLoopThread>();
    thr->run();

    auto loop = thr->getLoop();
    InetAddress addr("0.0.0.0:10000");
    std::shared_ptr<Acceptor> acceptor = std::make_shared<Acceptor>(loop, addr);
    acceptor->setAcceptCallback([](int sock, const InetAddress& addr) {
        NETLOG_INFO << "new connection from sock: " << sock << " addr: " << addr.toIpPort();
    });
    acceptor->start();

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    return 0;
}