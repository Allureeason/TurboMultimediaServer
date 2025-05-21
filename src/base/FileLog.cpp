#include "FileLog.h"

#include <iostream>
#include <cstring>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>

using namespace tmms::base;

// 文件权限
const int kFilePerm = 0666;

bool FileLog::open(const std::string& filePath) {
    filePath_ = filePath;
    fd_ = ::open(filePath.c_str(), O_RDWR | O_CREAT | O_APPEND, kFilePerm);
    if (fd_ < 0) {
        std::cout << "open file failed, filePath: " << filePath << ", error: " << strerror(errno) << std::endl;
        return false;
    }

    // std::cout << "open file success, filePath: " << filePath << ", fd: " << fd_ << std::endl;
    return true;
}

void FileLog::writeLog(const std::string& message) {
    int fd = fd_ == -1 ? 1 : fd_;
    ::write(fd, message.c_str(), message.size());
}

void FileLog::rotate(const std::string& filePath) {
    // 如果文件路径为空,直接返回
    if (filePath.empty()) {
        return;
    }

    // 重命名文件
    int ret = ::rename(filePath_.c_str(), filePath.c_str());
    if (ret < 0) {
        std::cout << "rename file failed, filePath: " << filePath << ", error: " << strerror(errno) << std::endl;
        return;
    }

    // 以读写、创建、追加方式打开文件
    int fd = ::open(filePath_.c_str(), O_RDWR | O_CREAT | O_APPEND, kFilePerm);
    if (fd < 0) {
        std::cout << "open file failed, filePath: " << filePath << ", error: " <<  (errno) << std::endl;
        return;
    }
   
    // 复制新文件描述符到旧的文件描述符
    ::dup2(fd, fd_);
    // 关闭新打开的文件描述符
    ::close(fd);
}

void FileLog::setRotateType(RotateType type) {
    rotateType_ = type;
}

RotateType FileLog::getRotateType() const {
    return rotateType_;
}

uint64_t FileLog::getFileSize() const {
    return ::lseek(fd_, 0, SEEK_END);
}

std::string FileLog::getFilePath() const {
    return filePath_;
}