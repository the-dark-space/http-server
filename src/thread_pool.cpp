#include "thread_pool.h"
#include <iostream>
#include <unistd.h>
#include "http_parser.h"
#include "file_handler.h"
#include <sys/socket.h>

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
        int clientSocket
) {

    {
        std::lock_guard<
                std::mutex
        > lock(queueMutex);

        tasks.push(clientSocket);
    }

    condition.notify_one();
}

int ThreadPool::dequeue() {

    std::unique_lock<
            std::mutex
    > lock(queueMutex);

    condition.wait(
            lock,

            [this]() {

                return !tasks.empty();
            }
    );

    int task =
            tasks.front();

    tasks.pop();

    return task;
}

ThreadPool::~ThreadPool() {

    {
        std::lock_guard<
                std::mutex
        > lock(queueMutex);

        stop = true;
    }

    condition.notify_all();

    for(auto& worker : workers) {

        worker.join();
    }
}

void ThreadPool::processRequest(
        int clientSocket
) {

    char buffer[4096] = {0};

    recv(
            clientSocket,
            buffer,
            sizeof(buffer),
            0
    );

    std::string request(buffer);

    HttpRequest httpRequest =
            HttpParser::parse(request);

    std::cout
            << "Worker "
            << std::this_thread::get_id()
            << " processing "
            << httpRequest.path
            << "\n";

    std::string filePath;

    if(httpRequest.path == "/") {

        filePath =
                "../static/index.html";
    }

    else if(
            httpRequest.path
            == "/about"
    ) {

        filePath =
                "../static/about.html";
    }

    else {

        filePath = "";
    }

    std::string body;

    std::string status;

    if(!filePath.empty()) {

        body =
                FileHandler::readFile(
                        filePath
                );

        if(body.empty()) {

            status =
                    "404 Not Found";

            body =
                    "<html><body>"
                    "<h1>404 File Not Found</h1>"
                    "</body></html>";
        }

        else {

            status = "200 OK";
        }
    }

    else {

        status = "404 Not Found";

        body =
                "<html><body>"
                "<h1>404 Route Not Found</h1>"
                "</body></html>";
    }

    std::string response =

            "HTTP/1.1 " + status + "\r\n"

            "Content-Type: text/html\r\n"

            "Content-Length: "

            + std::to_string(body.size())

            + "\r\n\r\n"

            + body;

    send(
            clientSocket,
            response.c_str(),
            response.size(),
            0
    );

    close(clientSocket);
}
