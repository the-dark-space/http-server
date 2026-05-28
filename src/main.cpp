#include "server.h"
#include "config_manager.h"
#include <csignal>
#include <memory>
#include <iostream>

std::unique_ptr<Server> serverPtr;

void signalHandler(
        int signal
) {

    std::cout
            << "\nGracefully shutting down server...\n";

    if(serverPtr) {

        serverPtr->stop();
    }
}

int main() {

    int port =
            ConfigManager::getPort();

    int threads =
            ConfigManager::getThreadCount();

    signal(SIGINT, signalHandler);

    serverPtr = std::make_unique<Server>(
        port,
        threads);

    serverPtr->start();

    return 0;
}