#include "file_handler.h"

#include <fstream>

#include <sstream>

std::vector<char>
FileHandler::readBinaryFile(
        const std::string& filePath
) {

    std::ifstream file(
            filePath,
            std::ios::binary
    );

    if(!file.is_open()) {

        return {};
    }

    return std::vector<char>(
            std::istreambuf_iterator<char>(file),
            std::istreambuf_iterator<char>()
    );
}

std::string FileHandler::readFile(
        const std::string& path
) {

    std::ifstream file(path);

    if(!file.is_open()) {

        return "";
    }

    std::stringstream buffer;

    buffer << file.rdbuf();

    return buffer.str();
}