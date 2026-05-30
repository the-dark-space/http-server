#pragma once

#include <string>

class ResponseBuilder {

public:

    static std::string buildHeader(
            const std::string& status,
            const std::string& contentType,
            size_t contentLength
    );
};