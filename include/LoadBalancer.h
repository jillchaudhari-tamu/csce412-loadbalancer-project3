#pragma once
#include <queue>
#include <vector>
#include <string>

#include "Config.h"
#include "Request.h"
#include "RequestFactory.h"
#include "WebServer.h"
#include "Logger.h"

/**
 * @class LoadBalancer
 * @brief Owns the request queue and manages a pool of web servers.
 */
class LoadBalancer {
public:
    LoadBalancer(const Config& cfg,
                 std::string name,
                 std::string logFile,
                 bool fillInitialQueue = true,
                 bool internalArrivals = true);

    // --- UML methods ---
    void addRequest(const Request& r);
    void dispatch();
    void scaleServers();
    void generateSummary();

    // tiny getters for Switch summary 
    const std::string& name() const { return name_; }
    int queueSize() const { return (int)q_.size(); }
    int serverCount() const { return (int)servers_.size(); }
    long long processed() const { return processed_; }
    long long dropped() const { return dropped_; }
    long long generatedRandom() const { return generatedRandom_; }
private:
    // config + randomness
    Config cfg_;
    RequestFactory factory_;

    // core state
    std::queue<Request> q_;
    std::vector<WebServer> servers_;

    // time tracking (for cooldown)
    int currentTime_ = 0;
    int cooldownRemaining_ = 0;

    // stats for logging/summary
    int startingQueueSize_ = 0;
    int endingQueueSize_ = 0;
    long long generatedRandom_ = 0;
    long long processed_ = 0;
    long long dropped_ = 0; // firewall later
    long long serversAdded_ = 0;
    long long serversRemoved_ = 0;
    int peakQueue_ = 0;
    int peakServers_ = 0;

    std::string name_;
    bool internalArrivals_ = true;

    // internal helpers (private)
    void initServers();
    void fillInitialQueue();
    void tickServers();
    void maybeGenerateRandomRequest(); // uses cfg_.newRequestProb
    void addServer();
    void removeServerIfPossible();

    bool isBlockedIP(const std::string& ip) const;

    Logger* logger_ = nullptr;
};