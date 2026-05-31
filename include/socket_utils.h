#pragma once

#include <string>

class SocketUtils
{
public:

    static bool sendAll(
        int clientSocket,
        const char* data,
        size_t size
    );
};