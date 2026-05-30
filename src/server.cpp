#include "server.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "thread_pool.h"

Server::Server(
    int port,

    int threadCount)

    : port(port),
      threadCount(threadCount),
      running(true)
{
}

void Server::start()
{
    serverSocket =
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

    ThreadPool pool(threadCount);
    while (running)
    {
        int clientSocket =
            accept(
                serverSocket,
                nullptr,
                nullptr);

        if (clientSocket < 0)
        {

            if (!running)
            {
                break;
            }

            std::cerr
                << "Accept failed\n";

            continue;
        }

        pool.enqueue(clientSocket);
    }

    close(serverSocket);
}
void Server::stop()
{

    running = false;

    close(serverSocket);
}