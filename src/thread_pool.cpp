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
#include "access_logger.h"

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

    bool shouldCloseConnection = false;

    setsockopt(

        clientSocket,

        SOL_SOCKET,

        SO_RCVTIMEO,

        &timeout,

        sizeof(timeout));

    while (true)
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

            shouldCloseConnection = true;

            goto cleanup;
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

            shouldCloseConnection = true;
            goto cleanup;
        }

        std::string request(
            buffer,
            bytesReceived);

        HttpRequest httpRequest =
            HttpParser::parse(request);

        if (httpRequest.method.empty())
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
            if (!httpRequest.keepAlive)
            {
                shouldCloseConnection = true;
            }

            if (!httpRequest.keepAlive)
            {
                shouldCloseConnection = true;
                goto cleanup;
            }

            continue;
        }

        if (httpRequest.method != "GET")
        {
            Logger::log(
                "WARN",
                "Unsupported HTTP method: " + httpRequest.method);

            std::string response =
                RequestHandler::getMethodNotAllowedResponse();

            AccessLogger::logRequest(
                httpRequest.method,
                httpRequest.path,
                "405 Method Not Allowed");

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
            if (!httpRequest.keepAlive)
            {
                shouldCloseConnection = true;
            }

            if (!httpRequest.keepAlive)
            {
                shouldCloseConnection = true;
                goto cleanup;
            }

            continue;
        }

        HttpResponse response =
            StaticFileHandler::serveFile(
                httpRequest.path);
        AccessLogger::logRequest(
            httpRequest.method,
            httpRequest.path,
            response.status);
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

        if (!httpRequest.keepAlive)
        {
            Logger::log(
                "INFO",
                "Client requested connection close");

            shouldCloseConnection = true;

            break;
        }
    }
cleanup:

    if (shouldCloseConnection)
    {
        Logger::log(
            "INFO",
            "Closing socket");

        close(clientSocket);
    }
}
