#pragma once
#include <vector>
#include <string>
#include "http_parser.h"

class RequestHandler
{
public:
        static std::string handleRequest(
            const HttpRequest &httpRequest);

        static std::string getMetricsResponse();

        static std::string getForbiddenResponse();

        static std::string getNotFoundResponse(
            const std::string &message);
        static std::string resolveFilePath(
            const std::string &requestPath);
        static bool isFileMissing(
            const std::string &body,
            const std::vector<char> &binaryBody);
        static std::string getMethodNotAllowedResponse();
};