#pragma once

#include <string>

class Router
{
public:
    static std::string resolvePath(
        const std::string &requestPath);
};