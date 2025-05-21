#include "FileLogMgr.h"
#include "TTime.h"
#include "FileLog.h"
#include "StringUtils.h"

#include <iostream>

using namespace tmms::base;

void FileLogMgr::onCheck() {
    // 获取当前时间
    int year = 0;
    int month = 0;
    int day = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;
    
    TTime::now(year, month, day, hour, minute, second);

    if (lastDay_ == -1) {
        lastDay_ = day;
    }

    if (lastHour_ == -1) {
        lastHour_ = hour;
    }

    if (lastMinute_ == -1) {
        lastMinute_ = minute;
    }
    
    bool dayChanged = lastDay_ != day;
    bool hourChanged = lastHour_ != hour;
    bool minuteChanged = lastMinute_ != minute;
    
    if (!dayChanged && !hourChanged && !minuteChanged) {
        return;
    }

    // 更新时间
    lastYear_ = year;
    lastMonth_ = month;


    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& l : logs_) {
        if (dayChanged && l.second->getRotateType() == kRotateDay) {
            rotateDays(l.second);
        }
        if (hourChanged && l.second->getRotateType() == kRotateHour) {
            rotateHours(l.second);
        }
        if (minuteChanged && l.second->getRotateType() == kRotateMinute) {
            rotateMinutes(l.second);
        }
    }

    lastDay_ = day;
    lastHour_ = hour;
    lastMinute_ = minute;
}


FileLogPtr FileLogMgr::getFileLog(const std::string& fileName) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = logs_.find(fileName);
    if (it != logs_.end()) {
        return it->second;
    }
    auto fileLog = std::make_shared<FileLog>();
    if (!fileLog->open(fileName)) {
        return nullptr;
    }
    logs_.emplace(fileName, fileLog);
    return fileLog;
}

void FileLogMgr::removeFileLog(const FileLogPtr& fileLog) {
    std::lock_guard<std::mutex> lock(mutex_);
    logs_.erase(fileLog->getFilePath());
}

void FileLogMgr::rotateDays(const FileLogPtr& fileLog) {
    if (fileLog->getFileSize() <= 0) {
        return;
    }

    std::cout << "rotateDays, fileLog: " << fileLog->getFilePath() << ", fileSize: " << fileLog->getFileSize() << std::endl;

    // 创建新文件
    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "_%04d%02d%02d", lastYear_, lastMonth_, lastDay_);

    std::string filePath = fileLog->getFilePath();
    std::string path = StringUtils::getFilePath(filePath);
    std::string fileName = StringUtils::getFileNameWithoutExt(filePath);
    std::string fileExt = StringUtils::getFileExt(filePath);
    std::string newFilePath = path + "/" + fileName + buf + "." + fileExt;

    fileLog->rotate(newFilePath);
}

void FileLogMgr::rotateHours(const FileLogPtr& fileLog) {
    if (fileLog->getFileSize() <= 0) {
        return;
    }

    // 创建新文件
    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "_%04d%02d%02dT%02d", lastYear_, lastMonth_, lastDay_, lastHour_);

    std::string filePath = fileLog->getFilePath();
    std::string path = StringUtils::getFilePath(filePath);
    std::string fileName = StringUtils::getFileNameWithoutExt(filePath);
    std::string fileExt = StringUtils::getFileExt(filePath);
    std::string newFilePath = path + "/" + fileName + buf + "." + fileExt;

    fileLog->rotate(newFilePath);
}

void FileLogMgr::rotateMinutes(const FileLogPtr& fileLog) {
    if (fileLog->getFileSize() <= 0) {
        return;
    }

    // 创建新文件
    char buf[128] = {0};
    snprintf(buf, sizeof(buf), "_%04d%02d%02dT%02d%02d", lastYear_, lastMonth_, lastDay_, lastHour_, lastMinute_);

    std::string filePath = fileLog->getFilePath();
    std::string path = StringUtils::getFilePath(filePath);
    std::string fileName = StringUtils::getFileNameWithoutExt(filePath);
    std::string fileExt = StringUtils::getFileExt(filePath);
    std::string newFilePath = path + "/" + fileName + buf + "." + fileExt;

    fileLog->rotate(newFilePath);
}