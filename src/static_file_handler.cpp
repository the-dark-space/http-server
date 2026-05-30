#include "static_file_handler.h"
#include "router.h"
#include "file_handler.h"
#include "mime_type.h"
#include "metrics_manager.h"
#include "request_handler.h"
#include "response_builder.h"

HttpResponse StaticFileHandler::serveFile(
    const std::string &requestPath)
{
    HttpResponse response;
    std::string filePath =
        Router::resolvePath(
            requestPath);

    if (filePath.empty())
    {
        MetricsManager::incrementNotFoundRequests();

        response.status =
            "404 Not Found";

        response.contentType =
            "text/html";

        response.body =
            "<html><body>"
            "<h1>404 Route Not Found</h1>"
            "</body></html>";

        return response;
    }

    std::string body;
    std::vector<char> binaryBody;

    if (
        MimeType::isBinaryFile(
            filePath))
    {
        binaryBody =
            FileHandler::readBinaryFile(
                filePath);
    }
    else
    {
        body =
            FileHandler::readFile(
                filePath);
    }
    bool fileMissing =

        body.empty()

        &&

        binaryBody.empty();

    if (fileMissing)
    {
        MetricsManager::incrementNotFoundRequests();

        response.status =
            "404 Not Found";

        response.contentType =
            "text/html";

        response.body =
            "<html><body>"
            "<h1>404 File Not Found</h1>"
            "</body></html>";

        return response;
    }

    MetricsManager::incrementSuccessfulRequests();

    std::string contentType =
        MimeType::getMimeType(
            filePath);

    response.status =
        "200 OK";

    response.contentType =
        contentType;

    if (binaryBody.empty())
    {
        response.body =
            body;

        response.isBinary =
            false;
    }
    else
    {
        response.binaryBody =
            binaryBody;

        response.isBinary =
            true;
    }

    return response;
}