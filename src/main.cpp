#include "server.h"
#include "config_manager.h"

int main() {

    int port =
            ConfigManager::getPort();

    int threads =
            ConfigManager::getThreadCount();

    Server server(
            port,
            threads
    );

    server.start();

    return 0;
}