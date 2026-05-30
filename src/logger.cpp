#include "logger.h"
#include <fstream>
#include <iostream>
#include <chrono>
#include <iomanip>

std::mutex Logger::logMutex;

void Logger::log(

    const std::string &level,

    const std::string &message)
{

        std::lock_guard<std::mutex>
            lock(logMutex);

        std::ofstream logFile(
            "../logs/server.log",

            std::ios::app);

        auto now =
            std::chrono::system_clock ::now();

        auto time =
            std::chrono::system_clock ::to_time_t(now);

        logFile
            << "[" << level << "] "

            << std::put_time(
                   std::localtime(&time),
                   "%Y-%m-%d %H:%M:%S")

            << " "

            << message

            << "\n";

        logFile.close();
}