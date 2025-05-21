#include "LogStream.h"
#include "TTime.h"

#include <cstring>
#include <sys/syscall.h>
#include <unistd.h>

using namespace tmms::base;

static thread_local pid_t t_thread_id = 0;
Logger* tmms::base::g_logger = nullptr;

const char* LogLevelStr[] = {
    "TRACE",
    "DEBUG",
    "INFO",
    "WARNING",
    "ERROR"
};


// 日志格式：时间 线程ID 日志等级 [文件:行号][函数名] 日志内容
LogStream::LogStream(Logger* logger, const char* file, int line, LogLevel level, const char* func)
    : logger_(logger) {
    // 时间
    auto now = tmms::base::TTime::IOSNowMs();
    stream_ << now << " ";

    // 线程ID
    if (t_thread_id == 0) {
        t_thread_id = static_cast<pid_t>(syscall(SYS_gettid));
    }
    stream_  << t_thread_id << " ";

    // 日志等级
    stream_ << LogLevelStr[level] << " ";

    // 文件名和行号
    const char* file_name = strrchr(file, '/');
    if (file_name) {
        file_name++;
    } else {
        file_name = file;
    }
    stream_ << "[" << file_name << ":" << line << "]" << " ";

    // 函数名
    stream_ << "[" << func << "] ";
}

LogStream::~LogStream() {
    stream_ << "\n";
    logger_->write(stream_.str());
}
