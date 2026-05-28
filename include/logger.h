#pragma once

#include <string>

#include <mutex>

class Logger {

public:

    static void log(
            const std::string& level,

            const std::string& message
    );

private:

    static std::mutex logMutex;
};