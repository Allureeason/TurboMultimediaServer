#ifndef STRING_UTILS_H
#define STRING_UTILS_H

#include <string>
#include <vector>

namespace tmms {
    namespace base {
        class StringUtils {
        public:
            // 去除字符串两端的空格
            static std::string trimSpaces(const std::string& str);
            // 去除字符串左端的空格
            static std::string trimLeftSpaces(const std::string& str);
            // 去除字符串右端的空格
            static std::string trimRightSpaces(const std::string& str);
            // 去除字符串两端的指定字符
            static std::string trim(const std::string& str, const std::string& chars);
            // 去除字符串左端的指定字符
            static std::string trimLeft(const std::string& str, const std::string& chars);
            // 去除字符串右端的指定字符
            static std::string trimRight(const std::string& str, const std::string& chars);
            // 将字符串转换为小写
            static std::string toLower(const std::string& str);
            // 将字符串转换为大写
            static std::string toUpper(const std::string& str);
            // 切分字符串
            static std::vector<std::string> split(const std::string& str, const std::string& delimiter);

            // 获取文件名
            static std::string getFileName(const std::string& filePath);
            // 获取文件名不包含后缀
            static std::string getFileNameWithoutExt(const std::string& filePath);
            // 获取文件后缀
            static std::string getFileExt(const std::string& filePath);
            // 获取文件路径
            static std::string getFilePath(const std::string& filePath);
            

        };
    } // namespace base
} // namespace tmms

#endif