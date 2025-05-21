#include "TTime.h"

#include <sys/time.h>
#include <time.h>

namespace tmms {
    namespace base {
        
        uint64_t TTime::now() {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return tv.tv_sec;
        }

        uint64_t TTime::nowMs() {
            struct timeval tv;
            gettimeofday(&tv, NULL);
            return tv.tv_sec * 1000 + tv.tv_usec / 1000;
        }

        void TTime::now(int& year, int& month, int& day, int& hour, int& minute, int& second) {
            struct tm tm;
            time_t t = time(NULL);
            localtime_r(&t, &tm);
            year = tm.tm_year + 1900;
            month = tm.tm_mon + 1;
            day = tm.tm_mday;
            hour = tm.tm_hour;
            minute = tm.tm_min;
            second = tm.tm_sec;
        }

        std::string TTime::ISONow() {
            struct tm tm;
            time_t t = time(NULL);
            localtime_r(&t, &tm);

            char buffer[100];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
            return std::string(buffer);
        }

        std::string TTime::IOSNowMs() {
            struct timeval tv;
            gettimeofday(&tv, NULL);

            struct tm tm;
            localtime_r(&tv.tv_sec, &tm);

            char buffer[100];
            strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S", &tm);
            // 显示三位毫秒
            char ms[4];
            snprintf(ms, sizeof(ms), "%03d", (int)(tv.tv_usec / 1000));
            return std::string(buffer) + "." + std::string(ms);
        }
    }
}