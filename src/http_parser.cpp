#include "http_parser.h"
#include <sstream>

HttpRequest HttpParser::parse(
    const std::string &request)
{

    HttpRequest httpRequest;

    std::stringstream stream(request);

    if (!(stream >> httpRequest.method >> httpRequest.path >> httpRequest.version))
    {
        return {};
    }
    std::string line;
    /*
     * Consume remainder of first line
     */
    std::getline(
        stream,
        line);

    while (std::getline(
        stream,
        line))
    {
        if (line == "\r" || line.empty())
        {
            break;
        }

        auto colonPos =
            line.find(':');

        if (colonPos ==
            std::string::npos)
        {
            continue;
        }

        std::string key =
            line.substr(
                0,
                colonPos);

        std::string value =
            line.substr(
                colonPos + 1);

        if (!value.empty() && value[0] == ' ')
        {
            value.erase(
                0,
                1);
        }

        if (!value.empty() && value.back() == '\r')
        {
            value.pop_back();
        }

        httpRequest.headers[key] =
            value;
    }
    auto it =
        httpRequest.headers.find(
            "Content-Length");

    if (it !=
        httpRequest.headers.end())
    {
        httpRequest.contentLength =
            std::stoul(
                it->second);
    }
    std::string body;

    std::getline(
        stream,
        body,
        '\0');

    httpRequest.body =
        body;

    if (httpRequest.version == "HTTP/1.1")
    {
        httpRequest.keepAlive = true;
    }

    auto connectionIt =
        httpRequest.headers.find(
            "Connection");

    if (connectionIt !=
        httpRequest.headers.end())
    {
        if (connectionIt->second == "close")
        {
            httpRequest.keepAlive =
                false;
        }
    }
    return httpRequest;
}