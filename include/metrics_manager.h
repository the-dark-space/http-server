#pragma once

#include <atomic>
#include <string>

class MetricsManager
{

public:
    static void incrementTotalRequests();

    static void incrementSuccessfulRequests();

    static void incrementNotFoundRequests();

    static std::string getMetrics();

    static void incrementActiveConnections();

    static void decrementActiveConnections();

    static void incrementActiveWorkers();

    static void decrementActiveWorkers();

private:
    static std::atomic<long> totalRequests;

    static std::atomic<long> successfulRequests;

    static std::atomic<long> notFoundRequests;

    static std::atomic<int> activeConnections;

    static std::atomic<int> activeWorkers;
};