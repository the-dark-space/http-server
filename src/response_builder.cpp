#include "response_builder.h"

std::string ResponseBuilder::buildHeader(
        const std::string& status,
        const std::string& contentType,
        size_t contentLength
) {

    return

        "HTTP/1.1 "
        + status
        + "\r\n"

        + "Content-Type: "
        + contentType
        + "\r\n"

        + "Content-Length: "
        + std::to_string(contentLength)
        + "\r\n\r\n";
}