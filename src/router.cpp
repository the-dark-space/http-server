#include "router.h"

std::unordered_map<
    RouteKey,
    Router::RouteHandler,
    RouteKeyHash>
    Router::routes;

std::string Router::resolvePath(
    const std::string &requestPath)
{
    if (requestPath == "/")
    {
        return "../static/index.html";
    }

    return "../static" + requestPath;
}

void Router::
    registerRoute(
        const std::string &method,
        const std::string &path,
        RouteHandler handler)
{
    routes[RouteKey{
        method,
        path}] = handler;
}

HttpResponse Router::
    handleRoute(
        const HttpRequest &request)
{
    RouteKey key{
        request.method,
        request.path};

    auto it =
        routes.find(
            key);

    if (it ==
        routes.end())
    {
        return {};
    }

    return it->second(
        request);
}