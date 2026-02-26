#pragma once
#include <queue>
#include <vector>
#include "Config.h"
#include "Request.h"
#include "RequestFactory.h"
#include "WebServer.h"
#include <string>

/**
 * @class LoadBalancer
 * @brief Owns the request queue and manages a pool of web servers.
 */
class LoadBalancer {
public:
    explicit LoadBalancer(const Config& cfg);

    // --- UML methods ---
    void addRequest(const Request& r);
    void dispatch();
    void scaleServers();
    void generateSummary();

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

    // internal helpers (private)
    void initServers();
    void fillInitialQueue();
    void tickServers();
    void maybeGenerateRandomRequest(); // uses cfg_.newRequestProb
    void addServer();
    void removeServerIfPossible();

    bool isBlockedIP(const std::string& ip) const;
};