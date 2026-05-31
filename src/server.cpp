#include "server.h"
#include <iostream>
#include <unistd.h>
#include <arpa/inet.h>
#include <cstring>
#include "thread_pool.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/epoll.h>
#include "metrics_manager.h"

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

    int flags =
        fcntl(
            serverSocket,
            F_GETFL,
            0);

    fcntl(
        serverSocket,
        F_SETFL,
        flags | O_NONBLOCK);

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

    int epollFd =
        epoll_create1(0);

    if (epollFd < 0)
    {
        std::cerr
            << "epoll_create1 failed\n";

        return;
    }

    epoll_event event;

    event.events =
        EPOLLIN;

    event.data.fd =
        serverSocket;

    if (
        epoll_ctl(
            epollFd,
            EPOLL_CTL_ADD,
            serverSocket,
            &event) < 0)
    {
        std::cerr
            << "epoll_ctl failed\n";

        return;
    }
    std::cout
        << "Server listening on port "
        << port
        << "\n";

    ThreadPool pool(threadCount);
    epoll_event events[10];
    while (running)
    {
        int readyCount =
            epoll_wait(
                epollFd,
                events,
                10,
                -1);

        if (readyCount < 0)
        {
            if (!running)
            {
                break;
            }

            continue;
        }

        for (
            int i = 0;
            i < readyCount;
            i++)
        {
            if (
                events[i].data.fd ==
                serverSocket)
            {
                int clientSocket =
                    accept(
                        serverSocket,
                        nullptr,
                        nullptr);

                if (clientSocket >= 0)
                {
                    MetricsManager::
                        incrementActiveConnections();
                    pool.enqueue(
                        clientSocket);
                }
            }
        }
    }

    close(serverSocket);
}
void Server::stop()
{

    running = false;

    close(serverSocket);
}