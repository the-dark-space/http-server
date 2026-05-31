#include "socket_utils.h"

#include <sys/socket.h>

bool SocketUtils::sendAll(
    int clientSocket,
    const char* data,
    size_t size)
{
    size_t totalSent = 0;

    while(totalSent < size)
    {
        ssize_t bytesSent =
            send(
                clientSocket,
                data + totalSent,
                size - totalSent,
                0);

        if(bytesSent <= 0)
        {
            return false;
        }

        totalSent += bytesSent;
    }

    return true;
}