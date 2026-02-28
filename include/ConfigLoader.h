#pragma once
#include <string>
#include "Config.h"

/**
 * @class ConfigLoader
 * @brief Loads configuration values from a simple key=value text file.
 */
class ConfigLoader {
public:
    /**
     * @brief Load config values from a file. Missing/invalid values keep defaults.
     * @return true if file opened successfully, false otherwise.
     */
    static bool loadFromFile(const std::string& path, Config& cfg);
};