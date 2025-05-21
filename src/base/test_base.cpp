#include <iostream>
#include <thread>
#include <chrono>

#include "TTime.h"
#include "StringUtils.h"
#include "Singleton.h"
#include "LogStream.h"
#include "FileLogMgr.h"
#include "FileLog.h"

// 测试字符串操作
void testStringUtils() {
    std::string str = "Hello, World!";
    std::string trimmed = tmms::base::StringUtils::trimSpaces(str);
    std::cout << "trimmed: " << trimmed << std::endl;

    std::string str2 = "   Hello, World!   ";
    std::string trimmed2 = tmms::base::StringUtils::trim(str2, " ");
    std::cout << "trimmed2: " << trimmed2 << std::endl;

    std::string str3 = "Hello, World!";
    std::string lowerStr = tmms::base::StringUtils::toLower(str3);
    std::cout << "lowerStr: " << lowerStr << std::endl;

    std::string str4 = "Hello, World!";
    std::string upperStr = tmms::base::StringUtils::toUpper(str4);
    std::cout << "upperStr: " << upperStr << std::endl;

    std::string str5 = "Hello, World!";
    std::string leftTrimmed = tmms::base::StringUtils::trimLeft(str5, " ");
    std::cout << "leftTrimmed: " << leftTrimmed << std::endl;

    std::string str6 = "Hello, World!";
    std::string rightTrimmed = tmms::base::StringUtils::trimRight(str6, " ");
    std::cout << "rightTrimmed: " << rightTrimmed << std::endl;

    std::string str7 = "Hello, World!";
    std::vector<std::string> splitResult = tmms::base::StringUtils::split(str7, ",");
    std::cout << "splitResult: ";
    for (const auto& part : splitResult) {
        std::cout << part << " ";
    }
    std::cout << std::endl;

    std::string str8 = "Hello, World!";
    std::string trimmed8 = tmms::base::StringUtils::trim(str8, " ");
    std::cout << "trimmed8: " << trimmed8 << std::endl;

    std::string str9 = "Hello, World!";
    std::string leftTrimmed9 = tmms::base::StringUtils::trimLeft(str9, " ");
    std::cout << "leftTrimmed9: " << leftTrimmed9 << std::endl;

    std::string str10 = "Hello, World!";
    std::string rightTrimmed10 = tmms::base::StringUtils::trimRight(str10, " ");
    std::cout << "rightTrimmed10: " << rightTrimmed10 << std::endl;
    

    std::string filePath = "/Users/xxx/Desktop/test.log";
    std::string path = tmms::base::StringUtils::getFilePath(filePath);
    std::string fileName = tmms::base::StringUtils::getFileName(filePath);
    std::string fileNameWithoutExt = tmms::base::StringUtils::getFileNameWithoutExt(filePath);
    std::string fileExt = tmms::base::StringUtils::getFileExt(filePath);
    std::cout << "path: " << path << std::endl;
    std::cout << "fileName: " << fileName << std::endl;
    std::cout << "fileNameWithoutExt: " << fileNameWithoutExt << std::endl;
    std::cout << "fileExt: " << fileExt << std::endl;
}

// 测试时间
void testTTime() {
    std::cout << "now: " << tmms::base::TTime::now() << std::endl;
    std::cout << "nowMs: " << tmms::base::TTime::nowMs() << std::endl;

    int year, month, day, hour, minute, second;
    tmms::base::TTime::now(year, month, day, hour, minute, second);
    std::cout << "year: " << year << std::endl;
    std::cout << "month: " << month << std::endl;
    std::cout << "day: " << day << std::endl;
    std::cout << "hour: " << hour << std::endl;
    std::cout << "minute: " << minute << std::endl;
    std::cout << "second: " << second << std::endl;

    std::cout << "ISONow: " << tmms::base::TTime::ISONow() << std::endl;
    std::cout << "IOSNowMs: " << tmms::base::TTime::IOSNowMs() << std::endl;
}

class A : public tmms::base::NonCopyable {
public:
    void print() {
        std::cout << "A print " << std::endl;
    }
};
#define sA tmms::base::Singleton<A>::getInstance()

// 测试单例模式
void testSingleton() {
    auto a1 = sA;
    auto a2 = sA;
    a1->print();
    a2->print();
}

// 测试日志
void testLogger() {
    tmms::base::g_logger = new tmms::base::Logger(nullptr);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);
    LOG_TRACE << "test trace";
    LOG_DEBUG << "test debug";
    LOG_INFO << "test info";
    LOG_WARN << "test warn";
    LOG_ERROR << "test error";
}

// 测试文件日志
void testFileLog() {
    // 获取文件日志管理器实例
    auto fileLogMgr = tmms::base::Singleton<tmms::base::FileLogMgr>::getInstance();
    // 获取日志文件对象
    auto fileLog = fileLogMgr->getFileLog("test.log");
    // 设置日志文件的轮转类型（假设kRotateMinute是FileLog类的静态成员或枚举值）
    fileLog->setRotateType(tmms::base::RotateType::kRotateMinute);

    tmms::base::g_logger = new tmms::base::Logger(fileLog);
    tmms::base::g_logger->setLevel(tmms::base::LogLevel::kDebug);
    
    int count = 0;
    while (true) {
        LOG_INFO << "test " << count++;
        fileLogMgr->onCheck();
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    std::cout << "test_base" << std::endl;
    // testTTime();
    // testStringUtils();
    // testSingleton();
    // testLogger();

    testFileLog();
    return 0;
}