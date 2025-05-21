#ifndef TMM_BASE_LOG_STREAM_H
#define TMM_BASE_LOG_STREAM_H

#include "Logger.h"

#include <sstream>

namespace tmms {
    namespace base {

        // 全局日志器
        extern Logger* g_logger;

        class LogStream {
        public:
            LogStream(Logger* logger, const char* file, int line, LogLevel level, const char* func);
            ~LogStream();

            template<typename T> LogStream& operator<<(const T& value) {
                stream_ << value;
                return *this;
            }
            
        private:
            Logger* logger_ {nullptr};
            std::ostringstream stream_;
        };

        #define LOG_TRACE \
            if (tmms::base::g_logger && tmms::base::g_logger->getLevel() <= tmms::base::LogLevel::kTrace) \
                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::LogLevel::kTrace, __func__)
        #define LOG_DEBUG \
            if (tmms::base::g_logger && tmms::base::g_logger->getLevel() <= tmms::base::LogLevel::kDebug) \
                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::LogLevel::kDebug, __func__)
        #define LOG_INFO \
            if (tmms::base::g_logger && tmms::base::g_logger->getLevel() <= tmms::base::LogLevel::kInfo) \
                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::LogLevel::kInfo, __func__)
        #define LOG_WARN \
            if (tmms::base::g_logger && tmms::base::g_logger->getLevel() <= tmms::base::LogLevel::kWarning) \
                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::LogLevel::kWarning, __func__)
        #define LOG_ERROR \
            if (tmms::base::g_logger && tmms::base::g_logger->getLevel() <= tmms::base::LogLevel::kError) \
                tmms::base::LogStream(tmms::base::g_logger, __FILE__, __LINE__, tmms::base::LogLevel::kError, __func__)
    }
}
#endif