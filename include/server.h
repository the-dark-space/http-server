#pragma once
#include <atomic>
class Server
{

public:
    Server(
        int port,

        int threadCount);

    void start();
    void stop();

private:
    int port;
    int threadCount;
    int serverSocket;
    std::atomic<bool> running;
};