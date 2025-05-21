#include "StringUtils.h"

using namespace tmms::base;

std::string StringUtils::trimSpaces(const std::string& str) {
    return trim(str, " ");
}

std::string StringUtils::trimLeftSpaces(const std::string& str) {
    return trimLeft(str, " ");
}

std::string StringUtils::trimRightSpaces(const std::string& str) {
    return trimRight(str, " ");
}

std::string StringUtils::trim(const std::string& str, const std::string& chars) {
    return trimLeft(trimRight(str, chars), chars);
}

std::string StringUtils::trimLeft(const std::string& str, const std::string& chars) {
    std::string::size_type pos = 0;
    while (pos < str.size() && chars.find(str[pos]) != std::string::npos) {
        pos++;
    }
    return str.substr(pos);
}

std::string StringUtils::trimRight(const std::string& str, const std::string& chars) {
    std::string::size_type pos = str.size();
    while (pos > 0 && chars.find(str[pos - 1]) != std::string::npos) {
        pos--;
    }
    return str.substr(0, pos);
}

std::string StringUtils::toLower(const std::string& str) {
    std::string lowerStr = str;
    for (char& c : lowerStr) {
        if (c >= 'A' && c <= 'Z') {
            c = c + 32;
        }
    }
    return lowerStr;
}

std::string StringUtils::toUpper(const std::string& str) {
    std::string upperStr = str;
    for (char& c : upperStr) {
        if (c >= 'a' && c <= 'z') {
            c = c - 32;
        }
    }
    return upperStr;
}

std::vector<std::string> StringUtils::split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    std::string::size_type start = 0;
    std::string::size_type end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
        end = str.find(delimiter, start);
    }
    if (start < str.size()) {
        result.push_back(str.substr(start));
    }
    return result;
}


std::string StringUtils::getFileName(const std::string& filePath) {
    std::string::size_type pos = filePath.find_last_of("/");
    if (pos == std::string::npos) {
        return filePath;
    }
    return filePath.substr(pos + 1);
}

std::string StringUtils::getFileNameWithoutExt(const std::string& filePath) {
    std::string fileName = getFileName(filePath);
    std::string::size_type pos = fileName.find_last_of(".");
    if (pos == std::string::npos) {
        return fileName;
    }
    return fileName.substr(0, pos);
}

std::string StringUtils::getFileExt(const std::string& filePath) {
    std::string fileName = getFileName(filePath);
    std::string::size_type pos = fileName.find_last_of(".");
    if (pos == std::string::npos) {
        return "";
    }
    return fileName.substr(pos + 1);
}

std::string StringUtils::getFilePath(const std::string& filePath) {
    std::string::size_type pos = filePath.find_last_of("/");
    if (pos == std::string::npos) {
        return ".";
    }
    return filePath.substr(0, pos);
}