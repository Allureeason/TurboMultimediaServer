#include "Logger.h"
#include <iostream>

using namespace tmms::base;

void Logger::setLevel(LogLevel level) {
    level_ = level;
}

LogLevel Logger::getLevel() {
    return level_;
}

void Logger::write(const std::string& message) {
    if (fileLog_) {
        fileLog_->writeLog(message);
    }
    else {
        std::cout << message;
    }
}
