#include "config_manager.h"

#include <fstream>

#include <nlohmann/json.hpp>

using json = nlohmann::json;

json loadConfig() {

    std::ifstream file(
            "../config/config.json"
    );

    json config;

    file >> config;

    return config;
}

int ConfigManager::getPort() {

    json config =
            loadConfig();

    return config["port"];
}

int ConfigManager::getThreadCount() {

    json config =
            loadConfig();

    return config["threads"];
}