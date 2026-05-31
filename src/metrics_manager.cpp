#include "metrics_manager.h"

std::atomic<long>
    MetricsManager::totalRequests(0);

std::atomic<long>
    MetricsManager::successfulRequests(0);

std::atomic<long>
    MetricsManager::notFoundRequests(0);

std::atomic<int>
    MetricsManager::activeConnections(0);

std::atomic<int>
    MetricsManager::activeWorkers(0);

void MetricsManager::incrementTotalRequests()
{

    totalRequests++;
}

void MetricsManager::incrementSuccessfulRequests()
{

    successfulRequests++;
}

void MetricsManager::incrementNotFoundRequests()
{

    notFoundRequests++;
}

std::string MetricsManager::getMetrics()
{

    return

        "total_requests " + std::to_string(totalRequests.load()) + "\n"

        + "successful_requests " + std::to_string(successfulRequests.load()) + "\n"

        + "not_found_requests " + std::to_string(notFoundRequests.load()) + "\n"

        +  "active_connections " + std::to_string(activeConnections.load()) + "\n"

        +  "active_workers " + std::to_string(activeWorkers.load()) + "\n";
}

void MetricsManager::
    incrementActiveConnections()
{
    activeConnections++;
}

void MetricsManager::
    decrementActiveConnections()
{
    activeConnections--;
}

void MetricsManager::
    incrementActiveWorkers()
{
    activeWorkers++;
}

void MetricsManager::
    decrementActiveWorkers()
{
    activeWorkers--;
}