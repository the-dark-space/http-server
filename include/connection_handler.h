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
    void configureSocket();

    bool readRequest(
        std::string &request);

    bool parseRequest(
        const std::string &request,
        HttpRequest &httpRequest);

    bool sendResponse(
        const HttpResponse &response);

    HttpResponse buildResponse(
        const HttpRequest &httpRequest);

    explicit ConnectionHandler(
        int clientSocket);

    void handle();

    void closeConnection();
};