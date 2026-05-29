#include "metrics_manager.h"

std::atomic<long>
MetricsManager::totalRequests(0);

std::atomic<long>
MetricsManager::successfulRequests(0);

std::atomic<long>
MetricsManager::notFoundRequests(0);

void MetricsManager::incrementTotalRequests() {

    totalRequests++;
}

void MetricsManager::incrementSuccessfulRequests() {

    successfulRequests++;
}

void MetricsManager::incrementNotFoundRequests() {

    notFoundRequests++;
}

std::string MetricsManager::getMetrics() {

    return

        "total_requests "
        + std::to_string(totalRequests.load())
        + "\n"

        + "successful_requests "
        + std::to_string(successfulRequests.load())
        + "\n"

        + "not_found_requests "
        + std::to_string(notFoundRequests.load())
        + "\n";
}