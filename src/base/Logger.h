#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "FileLog.h"

namespace tmms {
    namespace base {
        enum LogLevel {
            kTrace,
            kDebug,
            kInfo,
            kWarning,
            kError,
            kLevelMax
        };


        class Logger {
        public:
            Logger(FileLogPtr fileLog, const std::string& name = "default")
                : fileLog_(fileLog), name_(name) {}
                
            ~Logger() = default;

            void setLevel(LogLevel level);
            LogLevel getLevel();

            void write(const std::string& message);
        private:
            FileLogPtr fileLog_ { nullptr };
            LogLevel level_ { kDebug };
            std::string name_;
        };
    }
}
#endif


