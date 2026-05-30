#include "http_parser.h"

#include <sstream>

HttpRequest HttpParser::parse(
        const std::string& request
) {

    HttpRequest httpRequest;

    std::stringstream stream(request);

    if (!(stream
          >> httpRequest.method
          >> httpRequest.path
          >> httpRequest.version))
    {
        return {};
    }

    return httpRequest;
}