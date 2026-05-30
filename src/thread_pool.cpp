#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include "http_parser.h"
#include "file_handler.h"
#include <sys/socket.h>
#include "logger.h"
#include "mime_type.h"
#include "metrics_manager.h"
#include <sys/time.h>
#include <cerrno>
#include "response_builder.h"
#include "request_handler.h"

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

int ThreadPool::dequeue()
{

    std::unique_lock<
        std::mutex>
        lock(queueMutex);

    condition.wait(
        lock,

        [this]()
        {
            return !tasks.empty();
        });

    int task =
        tasks.front();

    tasks.pop();

    return task;
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

    if (bytesReceived < 0)
{
    if(errno == EAGAIN
       || errno == EWOULDBLOCK)
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
        request
    );

if(!handledResponse.empty())
{
    send(
        clientSocket,
        handledResponse.c_str(),
        handledResponse.size(),
        0);

    close(clientSocket);

    return;
}

    if (httpRequest.method != "GET")
    {
        Logger::log(
            "WARN",
            "Unsupported HTTP method");

        close(clientSocket);

        return;
    }

    std::string filePath =
    RequestHandler::resolveFilePath(
        httpRequest.path
    );

    std::string body;
    std::vector<char> binaryBody;
    std::string status;

    std::string contentType =
        "text/html";

    if (!filePath.empty())
    {

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
        contentType = MimeType::getMimeType(filePath);

        bool fileMissing =
    RequestHandler::isFileMissing(
        body,
        binaryBody
    );

        if (fileMissing)
        {
            MetricsManager ::incrementNotFoundRequests();
            std::string response =
    RequestHandler::getNotFoundResponse(
        "404 File Not Found"
    );

send(
    clientSocket,
    response.c_str(),
    response.size(),
    0);

close(clientSocket);

return;
        }

        else
        {

            status = "200 OK";
            MetricsManager ::incrementSuccessfulRequests();
        }
    }

    else
    {

        MetricsManager ::incrementNotFoundRequests();

        std::string response =
    RequestHandler::getNotFoundResponse(
        "404 Route Not Found"
    );

send(
    clientSocket,
    response.c_str(),
    response.size(),
    0);

close(clientSocket);

return;
    }

    std::string response =

        "HTTP/1.1 " + status + "\r\n"

                               "Content-Type: " +
        contentType + "\r\n"

                      "Content-Length: "

        + std::to_string(
 
              !binaryBody.empty()

                  ?

                  binaryBody.size()

                  :

                  body.size())

        + "\r\n\r\n"

        + body;

    Logger::log(
        "INFO",
        httpRequest.method + " " + httpRequest.path + " " + status);

    std::string header =
    ResponseBuilder::buildHeader(
        status,
        contentType,
        !binaryBody.empty()
            ? binaryBody.size()
            : body.size()
    );

    send(
        clientSocket,
        header.c_str(),
        header.size(),
        0);

    if (binaryBody.empty())
    {

        send(
            clientSocket,
            body.c_str(),
            body.size(),
            0);
    }

    else
    {

        send(
            clientSocket,
            binaryBody.data(),
            binaryBody.size(),
            0);
    }

    close(clientSocket);
}
