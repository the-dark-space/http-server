#include "http_parser.h"

#include <sstream>

HttpRequest HttpParser::parse(
        const std::string& request
) {

    std::stringstream stream(request);

    HttpRequest httpRequest;

    stream
            >> httpRequest.method
            >> httpRequest.path
            >> httpRequest.version;

    return httpRequest;
}