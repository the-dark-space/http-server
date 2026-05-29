#pragma once

#include <atomic>
#include <string>

class MetricsManager {

public:

    static void incrementTotalRequests();

    static void incrementSuccessfulRequests();

    static void incrementNotFoundRequests();

    static std::string getMetrics();

private:

    static std::atomic<long> totalRequests;

    static std::atomic<long> successfulRequests;

    static std::atomic<long> notFoundRequests;
};