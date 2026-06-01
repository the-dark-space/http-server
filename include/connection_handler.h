#pragma once
#include <string>
#include "http_parser.h"
#include "http_response.h"

class ConnectionHandler
{
private:

    int clientSocket;
    bool shouldCloseConnection = false;
public:
    static void configureSocket(
        int clientSocket);

    static bool readRequest(
        int clientSocket,
        std::string &request);

    static bool parseRequest(
        const std::string &request,
        HttpRequest &httpRequest);

    static bool sendResponse(
        int clientSocket,
        const HttpResponse &response);

    static HttpResponse buildResponse(
        const HttpRequest &httpRequest);

    explicit ConnectionHandler(
        int clientSocket);

    void handle();
};