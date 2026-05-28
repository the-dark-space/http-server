#pragma once

class Server {

public:

    Server(
        int port,

        int threadCount
);

    void start();

private:

    int port;
    int threadCount;
};