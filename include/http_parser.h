#pragma once
#include <unordered_map>
#include <string>

struct HttpRequest
{

    std::string method;

    std::string path;

    std::string version;

    bool keepAlive = false;

    std::unordered_map<
        std::string,
        std::string>
        headers;
    std::string body;
    size_t contentLength = 0;
};

class HttpParser
{

public:
    static HttpRequest parse(
        const std::string &request);
};
