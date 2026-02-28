#include "ConfigLoader.h"
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

static inline std::string trim(const std::string& s) {
    size_t start = 0;
    while (start < s.size() && std::isspace((unsigned char)s[start])) start++;
    size_t end = s.size();
    while (end > start && std::isspace((unsigned char)s[end - 1])) end--;
    return s.substr(start, end - start);
}

bool ConfigLoader::loadFromFile(const std::string& path, Config& cfg) {
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string line;
    while (std::getline(in, line)) {
        line = trim(line);
        if (line.empty()) continue;
        if (line[0] == '#') continue;

        size_t eq = line.find('=');
        if (eq == std::string::npos) continue;

        std::string key = trim(line.substr(0, eq));
        std::string val = trim(line.substr(eq + 1));

        try {
            if (key == "taskTimeMin") cfg.taskTimeMin = std::stoi(val);
            else if (key == "taskTimeMax") cfg.taskTimeMax = std::stoi(val);
            else if (key == "minQueuePerServer") cfg.minQueuePerServer = std::stoi(val);
            else if (key == "maxQueuePerServer") cfg.maxQueuePerServer = std::stoi(val);
            else if (key == "scaleCooldownN") cfg.scaleCooldownN = std::stoi(val);
            else if (key == "newRequestProb") cfg.newRequestProb = std::stod(val);
            else if (key == "blockedChancePercent") cfg.blockedChancePercent = std::stoi(val);
            else if (key == "logVerboseDrops") cfg.logVerboseDrops = std::stoi(val);
            else if (key == "logCheckpointInterval") cfg.logCheckpointInterval = std::stoi(val);
            // ignore unknown keys
        } catch (...) {
            // ignore bad values and keep defaults
        }
    }

    // basic sanity guards
    if (cfg.taskTimeMin < 1) cfg.taskTimeMin = 1;
    if (cfg.taskTimeMax < cfg.taskTimeMin) cfg.taskTimeMax = cfg.taskTimeMin;

    if (cfg.minQueuePerServer < 1) cfg.minQueuePerServer = 1;
    if (cfg.maxQueuePerServer < cfg.minQueuePerServer) cfg.maxQueuePerServer = cfg.minQueuePerServer;

    if (cfg.scaleCooldownN < 0) cfg.scaleCooldownN = 0;

    if (cfg.newRequestProb < 0.0) cfg.newRequestProb = 0.0;
    if (cfg.newRequestProb > 1.0) cfg.newRequestProb = 1.0;

    if (cfg.blockedChancePercent < 0) cfg.blockedChancePercent = 0;
    if (cfg.blockedChancePercent > 100) cfg.blockedChancePercent = 100;

    if (cfg.logVerboseDrops != 0) cfg.logVerboseDrops = 1;
    if (cfg.logCheckpointInterval < 1) cfg.logCheckpointInterval = 1;

    return true;
}