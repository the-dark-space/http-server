#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include "http_parser.h"
#include <sys/socket.h>
#include "logger.h"
#include <sys/time.h>
#include <cerrno>
#include "response_builder.h"
#include "http_response.h"
#include "access_logger.h"
#include "metrics_manager.h"
#include "socket_utils.h"
#include "connection_handler.h"

ThreadPool::ThreadPool(
    int threadCount) : stop(false)
{

    for (int i = 0;
         i < threadCount;
         i++)
    {

        workers.emplace_back([this]()
                             {

            while(true) {

                int clientSocket;

                {
                    std::unique_lock<
                            std::mutex
                    > lock(queueMutex);

                    condition.wait(
                            lock,

                            [this]() {

                                return
                                        !tasks.empty()
                                        || stop;
                            }
                    );

                    if(stop
                       && tasks.empty()) {

                        return;
                    }

                    clientSocket =
                            tasks.front();

                    tasks.pop();
                }

                processRequest(clientSocket);
            } });
    }
}

void ThreadPool::enqueue(
    int clientSocket)
{

    {
        std::lock_guard<
            std::mutex>
            lock(queueMutex);

        tasks.push(clientSocket);
    }

    condition.notify_one();
}

ThreadPool::~ThreadPool()
{

    {
        std::lock_guard<
            std::mutex>
            lock(queueMutex);

        stop = true;
    }

    condition.notify_all();

    for (auto &worker : workers)
    {

        worker.join();
    }
}

void ThreadPool::processRequest(
    int clientSocket)
{
    MetricsManager::incrementActiveWorkers();

    ConnectionHandler::
        configureSocket(
            clientSocket);
    bool shouldCloseConnection = false;

    while (true)
    {
        std::string request;

        if (
            !ConnectionHandler::
                readRequest(
                    clientSocket,
                    request))
        {
            shouldCloseConnection = true;

            goto cleanup;
        }
        HttpRequest httpRequest;

        if (
            !ConnectionHandler::
                parseRequest(
                    request,
                    httpRequest))
        {
            continue;
        }
        Logger::log(
            "INFO",
            httpRequest.method + " " + httpRequest.path + " " + httpRequest.version);

        Logger::log(
            "INFO",
            httpRequest.keepAlive
                ? "Keep-Alive requested"
                : "No Keep-Alive");

        HttpResponse response =
            ConnectionHandler::
                buildResponse(
                    httpRequest);

        AccessLogger::logRequest(
            httpRequest.method,
            httpRequest.path,
            response.status);

        if (
            !ConnectionHandler::
                sendResponse(
                    clientSocket,
                    response))
        {
            shouldCloseConnection = true;
            goto cleanup;
        }

        if (!httpRequest.keepAlive)
        {
            Logger::log(
                "INFO",
                "Client requested connection close");

            shouldCloseConnection = true;

            goto cleanup;
        }
    }
cleanup:

    MetricsManager::
        decrementActiveWorkers();

    MetricsManager::
        decrementActiveConnections();

    Logger::log(
        "INFO",
        "Closing socket");

    close(clientSocket);
}