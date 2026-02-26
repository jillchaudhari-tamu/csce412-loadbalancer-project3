#pragma once
#include <queue>
#include <vector>
#include "Config.h"
#include "Request.h"
#include "RequestFactory.h"
#include "WebServer.h"

/**
 * @brief Manages a queue of requests and a pool of web servers.
 */
class LoadBalancer {
public:
    explicit LoadBalancer(const Config& cfg);

    // Run the simulation for cfg.totalCycles
    void run();

    // For testing/summary
    int queueSize() const;
    int activeServerCount() const;

private:
    Config cfg_;
    RequestFactory factory_;

    std::queue<Request> q_;
    std::vector<WebServer> servers_;

    // internal helpers
    void initServers();
    void fillInitialQueue();
    void assignToIdleServers();
    void tickServers();
};