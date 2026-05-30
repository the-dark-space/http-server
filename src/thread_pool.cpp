#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include "http_parser.h"
#include <sys/socket.h>
#include "logger.h"
#include <sys/time.h>
#include <cerrno>
#include "response_builder.h"
#include "request_handler.h"
#include "http_response.h"
#include "static_file_handler.h"

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
    timeval timeout;

    timeout.tv_sec = 5;

    timeout.tv_usec = 0;
    setsockopt(

        clientSocket,

        SOL_SOCKET,

        SO_RCVTIMEO,

        &timeout,

        sizeof(timeout));
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

        close(clientSocket);

        return;
    }
    if (bytesReceived < 0)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK)
        {
            Logger::log(
                "INFO",
                "No data available yet");
        }
        else
        {
            Logger::log(
                "WARN",
                "recv failed");
        }

        close(clientSocket);

        return;
    }

    std::string request(
        buffer,
        bytesReceived);

    HttpRequest httpRequest =
        HttpParser::parse(request);

    std::string handledResponse =
        RequestHandler::handleRequest(
            httpRequest);

    if (!handledResponse.empty())
    {
        ssize_t bytesSent = send(
            clientSocket,
            handledResponse.c_str(),
            handledResponse.size(),
            0);

        if (bytesSent < 0)
        {
            Logger::log(
                "WARN",
                "Failed to send response");
        }
        close(clientSocket);

        return;
    }

    if (httpRequest.method != "GET")
    {
        Logger::log(
            "WARN",
            "Unsupported HTTP method: " + httpRequest.method);

        std::string response =
            RequestHandler::getMethodNotAllowedResponse();

        ssize_t bytesSent = send(
            clientSocket,
            response.c_str(),
            response.size(),
            0);

        if (bytesSent < 0)
        {
            Logger::log(
                "WARN",
                "Failed to send response");
        }
        close(clientSocket);

        return;
    }

    HttpResponse response =
        StaticFileHandler::serveFile(
            httpRequest.path);

    std::string header =
        ResponseBuilder::buildHeader(
            response.status,
            response.contentType,

            response.isBinary
                ? response.binaryBody.size()
                : response.body.size());

    ssize_t bytesSent = send(
        clientSocket,
        header.c_str(),
        header.size(),
        0);
    if (bytesSent < 0)
    {
        Logger::log(
            "WARN",
            "Failed to send response");
    }

    if (response.isBinary)
    {
        ssize_t bytesSent = send(
            clientSocket,
            response.binaryBody.data(),
            response.binaryBody.size(),
            0);
        if (bytesSent < 0)
        {
            Logger::log(
                "WARN",
                "Failed to send response");
        }
    }
    else
    {
        ssize_t bytesSent = send(
            clientSocket,
            response.body.c_str(),
            response.body.size(),
            0);
        if (bytesSent < 0)
        {
            Logger::log(
                "WARN",
                "Failed to send response");
        }
    }

    close(clientSocket);

    return;
}
