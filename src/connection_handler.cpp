#include "connection_handler.h"
#include "logger.h"
#include <cerrno>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include "http_parser.h"
#include "response_builder.h"
#include "socket_utils.h"
#include "request_handler.h"
#include "static_file_handler.h"

void ConnectionHandler::
    configureSocket(
        int clientSocket)
{
    timeval timeout;

    timeout.tv_sec = 5;

    timeout.tv_usec = 0;

    setsockopt(
        clientSocket,
        SOL_SOCKET,
        SO_RCVTIMEO,
        &timeout,
        sizeof(timeout));
}

bool ConnectionHandler::
    readRequest(
        int clientSocket,
        std::string &request)
{
    char buffer[4096] = {0};

    int bytesReceived =
        recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0);

    if (bytesReceived == 0)
    {
        Logger::log(
            "INFO",
            "Client disconnected");

        return false;
    }

    if (bytesReceived < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            Logger::log(
                "DEBUG",
                "Keep-Alive timeout");
        }
        else
        {
            Logger::log(
                "WARN",
                "recv failed");
        }

        return false;
    }

    request.assign(
        buffer,
        bytesReceived);

    return true;
}

bool ConnectionHandler::
    parseRequest(
        const std::string &request,
        HttpRequest &httpRequest)
{
    httpRequest =
        HttpParser::parse(
            request);

    if (httpRequest.method.empty())
    {
        return false;
    }

    return true;
}

bool ConnectionHandler::
    sendResponse(
        int clientSocket,
        const HttpResponse &response)
{
    std::string header =
        ResponseBuilder::buildHeader(
            response.status,
            response.contentType,

            response.isBinary
                ? response.binaryBody.size()
                : response.body.size());

    if (
        !SocketUtils::sendAll(
            clientSocket,
            header.c_str(),
            header.size()))
    {
        Logger::log(
            "WARN",
            "Failed to send header");

        return false;
    }

    if (response.isBinary)
    {
        if (
            !SocketUtils::sendAll(
                clientSocket,
                response.binaryBody.data(),
                response.binaryBody.size()))
        {
            Logger::log(
                "WARN",
                "Failed to send binary body");

            return false;
        }
    }
    else
    {
        if (
            !SocketUtils::sendAll(
                clientSocket,
                response.body.c_str(),
                response.body.size()))
        {
            Logger::log(
                "WARN",
                "Failed to send body");

            return false;
        }
    }

    return true;
}

HttpResponse ConnectionHandler::
    buildResponse(
        const HttpRequest &httpRequest)
{
    HttpResponse response;

    HttpResponse handledResponse =
        RequestHandler::handleRequest(
            httpRequest);

    if (!handledResponse.status.empty())
    {
        return handledResponse;
    }

    if (httpRequest.method != "GET")
    {
        return RequestHandler::
            getMethodNotAllowedResponse();
    }

    return StaticFileHandler::
        serveFile(
            httpRequest.path);
}