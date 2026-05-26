#pragma once

#include <string>

class FileHandler {

public:

    static std::string readFile(
            const std::string& path
    );
};