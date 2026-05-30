#pragma once

#include <string>
#include <vector>

class FileHandler
{
public:
        static std::string readFile(
            const std::string &filePath);

        static std::vector<char> readBinaryFile(
            const std::string &filePath);
};