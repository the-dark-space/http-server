#pragma once

#include <string>

class MimeType
{

public:
        static std::string getMimeType(
            const std::string &filePath);
        static bool isBinaryFile(
            const std::string &filePath);
};