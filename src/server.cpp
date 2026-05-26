#include "server.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "http_parser.h"
#include "file_handler.h"

Server::Server(int port)
    : port(port) {}

void Server::start()
{
    int serverSocket =
        socket(
            AF_INET,
            SOCK_STREAM,
            0);

    if (serverSocket < 0)
    {
        std::cerr
            << "Socket creation failed\n";

        return;
    }

    sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY;
    serverAddress.sin_port =
        htons(port);

    if (bind(
            serverSocket,

            (sockaddr *)&serverAddress,

            sizeof(serverAddress)) < 0)
    {

        std::cerr
            << "Bind failed\n";

        return;
    }

    if (listen(serverSocket, 10) < 0)
    {

        std::cerr
            << "Listen failed\n";

        return;
    }

    std::cout
        << "Server listening on port "
        << port
        << "\n";

    while (true)
    {

        int clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr);

        if (clientSocket < 0)
        {

            std::cerr
                << "Accept failed\n";

            continue;
        }

        char buffer[4096] = {0};

        recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0);

        std::string request(buffer);

        HttpRequest httpRequest =
            HttpParser::parse(request);

        std::cout
            << "Method: "
            << httpRequest.method
            << "\n";

        std::cout
            << "Path: "
            << httpRequest.path
            << "\n";

        std::string body;

        std::string status;

        std::string filePath;

        if (httpRequest.path == "/")
        {

            filePath = "../static/index.html";
        }

        else if (httpRequest.path == "/about")
        {

            filePath = "../static/about.html";
        }

        else
        {

            filePath = "";
        }

        if (!filePath.empty())
        {

            body =
                FileHandler::readFile(
                    filePath);

            if (body.empty())
            {

                status = "404 Not Found";

                body =
                    "<html><body>"
                    "<h1>404 File Not Found</h1>"
                    "</body></html>";
            }

            else
            {

                status = "200 OK";
            }
        }

        else
        {

            status = "404 Not Found";

            body =
                "<html><body>"
                "<h1>404 Route Not Found</h1>"
                "</body></html>";
        }

        std::string response =

            "HTTP/1.1 " + status + "\r\n"

                                   "Content-Type: text/html\r\n"

                                   "Content-Length: " +
            std::to_string(body.size())

            + "\r\n\r\n"

            + body;

        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0);

        close(clientSocket);
    }

    close(serverSocket);
}