#include "request_handler.h"
#include "http_parser.h"
#include "metrics_manager.h"
#include "path_validator.h"
#include "router.h"
#include "file_handler.h"
#include "mime_type.h"

HttpResponse RequestHandler::getMetricsResponse()
{
    HttpResponse response;

    response.status =
        "200 OK";

    response.contentType =
        "text/plain";

    response.body =
        MetricsManager::getMetrics();

    return response;
}

HttpResponse RequestHandler::getForbiddenResponse()
{
    HttpResponse response;

    response.status =
        "403 Forbidden";

    response.contentType =
        "text/html";

    response.body =
        "<html><body>"
        "<h1>403 Forbidden</h1>"
        "</body></html>";

    return response;
}

HttpResponse RequestHandler::getNotFoundResponse(
    const std::string &message)
{
    HttpResponse response;

    response.status =
        "404 Not Found";

    response.contentType =
        "text/html";

    response.body =
        "<html><body><h1>" + message +
        "</h1></body></html>";

    return response;
}

HttpResponse RequestHandler::handleRequest(
    const HttpRequest &httpRequest)
{

    HttpResponse routeResponse =
        Router::handleRoute(
            httpRequest);

    if (!routeResponse.status.empty())
    {
        return routeResponse;
    }

    if (httpRequest.method.empty())
    {
        return {};
    }

    MetricsManager::incrementTotalRequests();

    if (httpRequest.path == "/metrics")
    {
        return getMetricsResponse();
    }

    if (
        httpRequest.path ==
        "/echo")
    {
        HttpResponse response;

        response.status =
            "200 OK";

        response.contentType =
            "text/plain";

        response.body =
            httpRequest.body;

        return response;
    }

    if (httpRequest.path == "/api/ping")
    {
        HttpResponse response;

        response.status =
            "200 OK";

        response.contentType =
            "application/json";

        response.body =
            R"({"status":"ok"})";

        return response;
    }

    if (!PathValidator::isSafePath(
            httpRequest.path))
    {
        return getForbiddenResponse();
    }
    return {};
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

HttpResponse RequestHandler::getMethodNotAllowedResponse()
{
    HttpResponse response;

    response.status =
        "405 Method Not Allowed";

    response.contentType =
        "text/html";

    response.body =
        "<html><body>"
        "<h1>405 Method Not Allowed</h1>"
        "</body></html>";

    return response;
}