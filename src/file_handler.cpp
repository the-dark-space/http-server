#include "file_handler.h"

#include <fstream>

#include <sstream>

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