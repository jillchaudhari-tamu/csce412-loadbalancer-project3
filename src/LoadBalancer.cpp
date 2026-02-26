#include "LoadBalancer.h"
#include <iostream>

LoadBalancer::LoadBalancer(const Config& cfg)
    : cfg_(cfg), factory_(cfg_) {
    initServers();
    fillInitialQueue();
}

void LoadBalancer::initServers() {
    servers_.clear();
    servers_.reserve(cfg_.numServers);
    for (int i = 0; i < cfg_.numServers; i++) {
        servers_.emplace_back(i);
    }
}

void LoadBalancer::fillInitialQueue() {
    int initialCount = cfg_.numServers * cfg_.initialQueueMultiplier;
    for (int i = 0; i < initialCount; i++) {
        q_.push(factory_.makeRequest());
    }
}

void LoadBalancer::assignToIdleServers() {
    for (auto& s : servers_) {
        if (q_.empty()) return;
        if (!s.isBusy()) {
            s.assign(q_.front());
            q_.pop();
        }
    }
}

void LoadBalancer::tickServers() {
    for (auto& s : servers_) {
        s.tick();
    }
}

void LoadBalancer::run() {
    std::cout << "\n=== LoadBalancer run start ===\n";
    std::cout << "Starting queue size: " << q_.size() << "\n";
    std::cout << "Servers: " << servers_.size() << "\n";

    for (int cycle = 0; cycle < cfg_.totalCycles; cycle++) {
        assignToIdleServers();
        tickServers();

        // tiny status print every 1000 cycles (for now)
        if (cycle % 1000 == 0) {
            std::cout << "Cycle " << cycle
                      << " | queue=" << q_.size() << "\n";
        }
    }

    std::cout << "Ending queue size: " << q_.size() << "\n";
    std::cout << "=== LoadBalancer run end ===\n\n";
}

int LoadBalancer::queueSize() const { return (int)q_.size(); }
int LoadBalancer::activeServerCount() const { return (int)servers_.size(); }