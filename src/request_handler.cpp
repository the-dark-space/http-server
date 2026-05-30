#include "request_handler.h"
#include "http_parser.h"
#include "metrics_manager.h"
#include "path_validator.h"
#include "router.h"
#include "file_handler.h"
#include "mime_type.h"
#include "response_builder.h"

std::string RequestHandler::getMetricsResponse()
{
    std::string body =
        MetricsManager::getMetrics();

    return ResponseBuilder::buildHeader(
               "200 OK",
               "text/plain",
               body.size()) +
           body;
}

std::string RequestHandler::getForbiddenResponse()
{
    std::string body =

        "<html><body>"
        "<h1>403 Forbidden</h1>"
        "</body></html>";

    return ResponseBuilder::buildHeader(
               "403 Forbidden",
               "text/html",
               body.size()) +
           body;
}

std::string RequestHandler::getNotFoundResponse(
    const std::string &message)
{
    std::string body =

        "<html><body><h1>" + message + "</h1></body></html>";

    return ResponseBuilder::buildHeader(
               "404 Not Found",
               "text/html",
               body.size()) +
           body;
}

std::string RequestHandler::handleRequest(
    const HttpRequest &httpRequest)
{

    if (httpRequest.method.empty())
    {
        return "";
    }

    MetricsManager::incrementTotalRequests();

    if (httpRequest.path == "/metrics")
    {
        return getMetricsResponse();
    }
    if (!PathValidator::isSafePath(
            httpRequest.path))
    {
        return getForbiddenResponse();
    }
    return "";
}

std::string RequestHandler::resolveFilePath(
    const std::string &requestPath)
{
    return Router::resolvePath(
        requestPath);
}

bool RequestHandler::isFileMissing(
    const std::string &body,
    const std::vector<char> &binaryBody)
{
    return body.empty() &&
           binaryBody.empty();
}

std::string RequestHandler::getMethodNotAllowedResponse()
{
    std::string body =

        "<html><body>"
        "<h1>405 Method Not Allowed</h1>"
        "</body></html>";

    return ResponseBuilder::buildHeader(
               "405 Method Not Allowed",
               "text/html",
               body.size()) +
           body;
}