#pragma once

#include "http_response.h"

class StaticFileHandler
{
public:
    static HttpResponse serveFile(
        const std::string &requestPath);
};