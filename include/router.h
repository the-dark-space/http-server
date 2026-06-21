#pragma once
#include <functional>
#include <unordered_map>
#include "http_parser.h"
#include "http_response.h"
#include <string>

struct RouteKey
{
    std::string method;

    std::string path;

    bool operator==(
        const RouteKey &other) const
    {
        return method == other.method && path == other.path;
    }
};

struct RouteKeyHash
{
    size_t operator()(
        const RouteKey &key) const
    {
        return std::hash<std::string>{}(key.method) ^ std::hash<std::string>{}(key.path);
    }
};

class Router
{
private:
    using RouteHandler =
        std::function<
            HttpResponse(
                const HttpRequest &)>;

    static std::unordered_map<
        RouteKey,
        RouteHandler,
        RouteKeyHash>
        routes;

public:
    static std::string resolvePath(
        const std::string &requestPath);

    static void registerRoute(
        const std::string &method,
        const std::string &path,
        RouteHandler handler);

    static HttpResponse handleRoute(
        const HttpRequest &request);
};
