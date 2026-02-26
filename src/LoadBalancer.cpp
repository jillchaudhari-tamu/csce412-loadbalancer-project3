#include "LoadBalancer.h"
#include <iostream>
#include <random>

LoadBalancer::LoadBalancer(const Config& cfg)
    : cfg_(cfg), factory_(cfg_) {
    initServers();
    fillInitialQueue();
    startingQueueSize_ = (int)q_.size();
    peakQueue_ = startingQueueSize_;
    peakServers_ = (int)servers_.size();
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

void LoadBalancer::addRequest(const Request& r) {
    if (isBlockedIP(r.ip_in)) {
        dropped_++;
        return;
    }
    q_.push(r);
}

void LoadBalancer::maybeGenerateRandomRequest() {
    static std::mt19937 rng(std::random_device{}());
    std::bernoulli_distribution coin(cfg_.newRequestProb);

    if (coin(rng)) {
        addRequest(factory_.makeRequest());
        generatedRandom_++;
    }
}

void LoadBalancer::dispatch() {
    currentTime_++;
    // 1) maybe generate new request for this cycle
    maybeGenerateRandomRequest();

    // 2) assign queued requests to idle servers
    for (auto& s : servers_) {
        if (q_.empty()) break;
        if (s.isIdle()) {
            Request r = q_.front();
            q_.pop();
            s.assign(r);
        }
    }

    // 3) process one cycle on each server
    tickServers();

    // update peak queue after changes this cycle
    if ((int)q_.size() > peakQueue_) peakQueue_ = (int)q_.size();
}

void LoadBalancer::tickServers() {
    for (auto& s : servers_) {
        bool wasBusy = !s.isIdle();
        s.tick();
        bool nowIdle = s.isIdle();

        // If it was busy and became idle after tick, one request finished
        if (wasBusy && nowIdle) {
            processed_++;
        }
    }
}

void LoadBalancer::addServer() {
    int newId = (int)servers_.size();
    servers_.emplace_back(newId);
    serversAdded_++;
}

void LoadBalancer::removeServerIfPossible() {
    if (servers_.size() <= 1) return;

    WebServer& last = servers_.back();
    if (!last.isIdle()) return; // don't remove busy server

    servers_.pop_back();
    serversRemoved_++;
}

void LoadBalancer::scaleServers() {
    // scaling is based on current queue size and current server count
    int sCount = (int)servers_.size();
    int qSize = (int)q_.size();

    int lower = cfg_.minQueuePerServer * sCount;
    int upper = cfg_.maxQueuePerServer * sCount;

    if (cooldownRemaining_ > 0) {
        cooldownRemaining_--;
        return;
    }

    if (qSize > upper) {
        addServer();
        std::cout << "[Scale Up] time=" << currentTime_
                  << " queue=" << q_.size()
                  << " servers=" << servers_.size() << "\n";
        cooldownRemaining_ = cfg_.scaleCooldownN;
    } else if (qSize < lower) {
        removeServerIfPossible();
        std::cout << "[Scale Down] time=" << currentTime_
                  << " queue=" << q_.size()
                  << " servers=" << servers_.size() << "\n";
        cooldownRemaining_ = cfg_.scaleCooldownN;
    }

    if ((int)servers_.size() > peakServers_) peakServers_ = (int)servers_.size();
}

void LoadBalancer::generateSummary() {
    endingQueueSize_ = (int)q_.size();

    std::cout << "\n=== Summary ===\n";
    std::cout << "Starting queue size: " << startingQueueSize_ << "\n";
    std::cout << "Ending queue size: " << endingQueueSize_ << "\n";
    std::cout << "Task time range: [" << cfg_.taskTimeMin << ", " << cfg_.taskTimeMax << "]\n";
    std::cout << "Random requests generated: " << generatedRandom_ << "\n";
    std::cout << "Processed requests: " << processed_ << "\n";
    std::cout << "Dropped (firewall) requests: " << dropped_ << "\n";
    std::cout << "Servers added: " << serversAdded_ << "\n";
    std::cout << "Servers removed: " << serversRemoved_ << "\n";
    std::cout << "Peak servers: " << peakServers_ << "\n";
    std::cout << "Peak queue size: " << peakQueue_ << "\n";
    std::cout << "=============\n\n";
}

bool LoadBalancer::isBlockedIP(const std::string& ip) const {
    // Block common private ranges (demo firewall)
    if (ip.rfind("10.", 0) == 0) return true;
    if (ip.rfind("192.168.", 0) == 0) return true;

    // block 172.16.0.0 - 172.31.255.255
    if (ip.rfind("172.", 0) == 0) {
        // parse second octet
        size_t firstDot = ip.find('.');
        if (firstDot != std::string::npos) {
            size_t secondDot = ip.find('.', firstDot + 1);
            if (secondDot != std::string::npos) {
                int second = std::stoi(ip.substr(firstDot + 1, secondDot - (firstDot + 1)));
                if (second >= 16 && second <= 31) return true;
            }
        }
    }

    return false;
}