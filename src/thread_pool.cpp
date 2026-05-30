#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include "http_parser.h"
#include "file_handler.h"
#include <sys/socket.h>
#include "logger.h"
#include "mime_type.h"
#include "path_validator.h"
#include "metrics_manager.h"
#include <sys/time.h>
#include <cerrno>

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
        Logger::log(
            "WARN",
            "Socket timeout or recv error");

        close(clientSocket);
        return;
    }

    std::string request(
        buffer,
        bytesReceived);

    HttpRequest httpRequest =
        HttpParser::parse(request);

    if (httpRequest.method != "GET")
    {
        Logger::log(
            "WARN",
            "Unsupported HTTP method");

        close(clientSocket);

        return;
    }
    MetricsManager::incrementTotalRequests();

    if (!PathValidator::isSafePath(
            httpRequest.path))
    {

        Logger::log(
            "WARN",
            "Blocked path traversal attempt: " + httpRequest.path);

        std::string response =

            "HTTP/1.1 403 Forbidden\r\n"

            "Content-Type: text/html\r\n"

            "\r\n"

            "<html><body>"
            "<h1>403 Forbidden</h1>"
            "</body></html>";

        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0);

        close(clientSocket);

        return;
    }

    if (httpRequest.path == "/metrics")
    {

        std::string body =
            MetricsManager::getMetrics();

        std::string response =

            "HTTP/1.1 200 OK\r\n"

            "Content-Type: text/plain\r\n"

            "Content-Length: "

            + std::to_string(body.size())

            + "\r\n\r\n"

            + body;

        send(
            clientSocket,
            response.c_str(),
            response.size(),
            0);

        close(clientSocket);

        return;
    }

    std::string filePath;

    if (httpRequest.path == "/")
    {

        filePath =
            "../static/index.html";
    }
    else
    {

        filePath =
            "../static" + httpRequest.path;
    }

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

            body.empty()

            &&

            binaryBody.empty();

        if (fileMissing)
        {

            status =
                "404 Not Found";
            MetricsManager ::incrementNotFoundRequests();
            body =
                "<html><body>"
                "<h1>404 File Not Found</h1>"
                "</body></html>";
        }

        else
        {

            status = "200 OK";
            MetricsManager ::incrementSuccessfulRequests();
        }
    }

    else
    {

        status = "404 Not Found";
        MetricsManager ::incrementNotFoundRequests();

        body =
            "<html><body>"
            "<h1>404 Route Not Found</h1>"
            "</body></html>";
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

        "HTTP/1.1 "

        + status

        + "\r\n"

        + "Content-Type: "

        + contentType

        + "\r\n"

        + "Content-Length: "

        + std::to_string(
              !binaryBody.empty()
                  ? binaryBody.size()
                  : body.size())

        + "\r\n\r\n";

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
