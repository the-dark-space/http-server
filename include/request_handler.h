#pragma once
#include <vector>
#include <string>
#include "http_parser.h"
#include "http_response.h"

class RequestHandler
{
public:
    static HttpResponse handleRequest(
        const HttpRequest &request);

    static HttpResponse getMetricsResponse();

    static HttpResponse getForbiddenResponse();

    static HttpResponse getNotFoundResponse(
        const std::string &message);
    static std::string resolveFilePath(
        const std::string &requestPath);
    static bool isFileMissing(
        const std::string &body,
        const std::vector<char> &binaryBody);
    static HttpResponse getMethodNotAllowedResponse();
};