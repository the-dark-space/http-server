#include "access_logger.h"
#include "logger.h"

void AccessLogger::logRequest(
        const std::string& method,
        const std::string& path,
        const std::string& status
)
{
    Logger::log(
        "INFO",
        method + " "
        + path + " "
        + status
    );
}