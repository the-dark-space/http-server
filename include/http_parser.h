#pragma once

#include <string>

struct HttpRequest {

    std::string method;

    std::string path;

    std::string version;
};

class HttpParser {

public:

    static HttpRequest parse(
            const std::string& request
    );
};