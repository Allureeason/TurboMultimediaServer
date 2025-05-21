#ifndef FILE_LOG_MGR_H
#define FILE_LOG_MGR_H

#include "Singleton.h"
#include "NonCopyable.h"
#include "FileLog.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <mutex>


namespace tmms {
    namespace base {

        using FileLogPtr = std::shared_ptr<FileLog>;

        // 文件日志管理器
        class FileLogMgr : public NonCopyable {
        public:
            FileLogMgr() = default;
            ~FileLogMgr() = default;

            void onCheck();
            FileLogPtr getFileLog(const std::string& fileName);
            void removeFileLog(const FileLogPtr& fileLog);
            void rotateDays(const FileLogPtr& fileLog);
            void rotateHours(const FileLogPtr& fileLog);
            void rotateMinutes(const FileLogPtr& fileLog);
        private:
            std::unordered_map<std::string, FileLogPtr> logs_;
            std::mutex mutex_;
            int lastYear_ = -1;
            int lastMonth_ = -1;
            int lastDay_ = -1;
            int lastHour_ = -1;
            int lastMinute_ = -1;
        };

        #define sFileLogMgr tmms::base::Singleton<tmms::base::FileLogMgr>::getInstance()
    }
}
#endif