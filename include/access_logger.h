#pragma once

#include <string>

class AccessLogger
{
public:

    static void logRequest(
            const std::string& method,
            const std::string& path,
            const std::string& status
    );
};