#include "router.h"

std::string Router::resolvePath(
        const std::string& requestPath
)
{
    if(requestPath == "/")
    {
        return "../static/index.html";
    }

    return "../static" + requestPath;
}