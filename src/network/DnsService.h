#ifndef DNS_SERVICE_H
#define DNS_SERVICE_H

#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "base/NonCopyable.h"
#include "network/net/InetAddress.h"

namespace tmms {
    namespace network {

        using InetAddressPtr = std::shared_ptr<InetAddress>;

        class DnsService : public base::NonCopyable {
        public:
            DnsService();
            ~DnsService();
            

            void addHost(const std::string& host);
            InetAddressPtr getHostAddress(const std::string& host, int index = 0);
            std::vector<InetAddressPtr> getHostAddresses(const std::string& host);
            void updateHostAddress(const std::string& host, std::vector<InetAddressPtr>& addrs);
            std::unordered_map<std::string, std::vector<InetAddressPtr>> getHostInfos();
            void setDnsServiceParams(int32_t interval, int32_t sleep, int32_t retry);

            void start();
            void stop();

            static void getHostInfo(const std::string& host, std::vector<InetAddressPtr>& addrs);

        private:
            void onWorker();

        private:
            std::unordered_map<std::string, std::vector<InetAddressPtr>> host_infos_;
            std::mutex mtx_;

            std::thread thread_;
            int32_t interval_ {5000};
            int32_t sleep_ {1000};
            int32_t retry_ {3};
            bool running_ {false};

        };
    }
}

#endif