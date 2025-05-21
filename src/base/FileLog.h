#ifndef FILE_LOGGER_H
#define FILE_LOGGER_H

#include <string>
#include <cstdint>
#include <memory>

namespace tmms {
    namespace base {
        enum RotateType {
            kNoRotate,
            kRotateHour,
            kRotateDay,
            kRotateMinute,
        };

        // 文件日志器
        class FileLog {
        public:
            FileLog() = default;
            ~FileLog() = default;

            bool open(const std::string& filePath);
            void writeLog(const std::string& message);
            void rotate(const std::string& filePath);
            void setRotateType(RotateType type);
            RotateType getRotateType() const;
            uint64_t getFileSize() const;
            std::string getFilePath() const;
            
        private:
            int fd_ {-1};
            std::string filePath_;
            RotateType rotateType_ {kNoRotate};
        };

        using FileLogPtr = std::shared_ptr<FileLog>;
    }
}

#endif
