#pragma once

#include <string>
#include <vector>

struct HttpResponse
{
    std::string status;

    std::string contentType;

    std::string body;

    std::vector<char> binaryBody;

    bool isBinary = false;
};