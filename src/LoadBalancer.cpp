#include "LoadBalancer.h"
#include <iostream>
#include <random>
#include <string>
#include "ConsoleColor.h"

// -------------------- constructor --------------------

LoadBalancer::LoadBalancer(const Config& cfg)
    : cfg_(cfg), factory_(cfg_) {
    initServers();
    fillInitialQueue();

    startingQueueSize_ = (int)q_.size();
    peakQueue_ = startingQueueSize_;
    peakServers_ = (int)servers_.size();

    // open log file
    logger_ = new Logger("logs/loadbalancer_log.txt");
    logger_->logLine("=== Load Balancer Log Start ===");
    logger_->logLine("Starting queue size: " + std::to_string(startingQueueSize_));
    logger_->logLine("Task time range: [" + std::to_string(cfg_.taskTimeMin) + ", " +
                     std::to_string(cfg_.taskTimeMax) + "]");
    logger_->logLine("Initial servers: " + std::to_string((int)servers_.size()));
    logger_->logLine("Queue thresholds: " +
                     std::to_string(cfg_.minQueuePerServer * (int)servers_.size()) + " to " +
                     std::to_string(cfg_.maxQueuePerServer * (int)servers_.size()));
    logger_->logLine("Scale cooldown (n): " + std::to_string(cfg_.scaleCooldownN));
    logger_->logLine("New request probability/cycle: " + std::to_string(cfg_.newRequestProb));
}

// -------------------- private helpers --------------------

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

bool LoadBalancer::isBlockedIP(const std::string& ip) const {
    // Block common private ranges (demo firewall / DOS prevention)
    if (ip.rfind("10.", 0) == 0) return true;
    if (ip.rfind("192.168.", 0) == 0) return true;

    // Optional: 172.16.0.0 - 172.31.255.255
    if (ip.rfind("172.", 0) == 0) {
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

void LoadBalancer::maybeGenerateRandomRequest() {
    static std::mt19937 rng(std::random_device{}());
    std::bernoulli_distribution coin(cfg_.newRequestProb);

    if (coin(rng)) {
        addRequest(factory_.makeRequest());  // firewall handled inside addRequest
        generatedRandom_++;
    }
}

void LoadBalancer::tickServers() {
    for (auto& s : servers_) {
        bool wasBusy = !s.isIdle();
        s.tick();
        bool nowIdle = s.isIdle();

        // finished a request this tick
        if (wasBusy && nowIdle) {
            processed_++;
        }
    }
}

void LoadBalancer::addServer() {
    int newId = (int)servers_.size();
    servers_.emplace_back(newId);
    serversAdded_++;

    std::cout << ConsoleColor::wrap(
        cfg_.useColor,
        ConsoleColor::GREEN,
        "[Scale Up] time=" + std::to_string(currentTime_) +
        " queue=" + std::to_string((int)q_.size()) +
        " servers=" + std::to_string((int)servers_.size())
    ) << "\n";

    if (logger_) {
        logger_->logLine("[Scale Up] time=" + std::to_string(currentTime_) +
                         " queue=" + std::to_string((int)q_.size()) +
                         " servers=" + std::to_string((int)servers_.size()));
    }
}

void LoadBalancer::removeServerIfPossible() {
    if (servers_.size() <= 1) return;

    WebServer& last = servers_.back();
    if (!last.isIdle()) return; // never remove busy server

    servers_.pop_back();
    serversRemoved_++;

    std::cout << ConsoleColor::wrap(
        cfg_.useColor,
        ConsoleColor::YELLOW,
        "[Scale Down] time=" + std::to_string(currentTime_) +
        " queue=" + std::to_string((int)q_.size()) +
        " servers=" + std::to_string((int)servers_.size())
    ) << "\n";

    if (logger_) {
        logger_->logLine("[Scale Down] time=" + std::to_string(currentTime_) +
                         " queue=" + std::to_string((int)q_.size()) +
                         " servers=" + std::to_string((int)servers_.size()));
    }
}

// -------------------- UML public methods --------------------

void LoadBalancer::addRequest(const Request& r) {
    if (isBlockedIP(r.ip_in)) {
        dropped_++;

        if (dropped_ % 50 == 0) {
            std::cout << ConsoleColor::wrap(
                cfg_.useColor,
                ConsoleColor::RED,
                "[Dropped] time=" + std::to_string(currentTime_) +
                " total_dropped=" + std::to_string(dropped_)
            ) << "\n";
        }

        if (logger_ && (dropped_ % 50 == 0)) {
            logger_->logLine("[Dropped] time=" + std::to_string(currentTime_) +
                            " total_dropped=" + std::to_string(dropped_));
        }
        return;
    }
    q_.push(r);
}

void LoadBalancer::dispatch() {
    // Track time internally (strict UML: Simulation doesn't set LB time directly)
    currentTime_++;

    // 1) new request(s) arriving randomly this cycle
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

    // 3) process one clock cycle on each server
    tickServers();

    // update peak queue size after all actions this cycle
    if ((int)q_.size() > peakQueue_) peakQueue_ = (int)q_.size();
}

void LoadBalancer::scaleServers() {
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
        cooldownRemaining_ = cfg_.scaleCooldownN;
    } else if (qSize < lower) {
        removeServerIfPossible();
        cooldownRemaining_ = cfg_.scaleCooldownN;
    }

    if ((int)servers_.size() > peakServers_) peakServers_ = (int)servers_.size();
}

void LoadBalancer::generateSummary() {
    endingQueueSize_ = (int)q_.size();

    // console summary
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

    // log summary (file)
    if (logger_) {
        logger_->logLine("=== Summary ===");
        logger_->logLine("Starting queue size: " + std::to_string(startingQueueSize_));
        logger_->logLine("Ending queue size: " + std::to_string(endingQueueSize_));
        logger_->logLine("Task time range: [" + std::to_string(cfg_.taskTimeMin) + ", " +
                         std::to_string(cfg_.taskTimeMax) + "]");
        logger_->logLine("Random requests generated: " + std::to_string(generatedRandom_));
        logger_->logLine("Processed requests: " + std::to_string(processed_));
        logger_->logLine("Dropped (firewall) requests: " + std::to_string(dropped_));
        logger_->logLine("Servers added: " + std::to_string(serversAdded_));
        logger_->logLine("Servers removed: " + std::to_string(serversRemoved_));
        logger_->logLine("Peak servers: " + std::to_string(peakServers_));
        logger_->logLine("Peak queue size: " + std::to_string(peakQueue_));
        logger_->logLine("=== Load Balancer Log End ===");
    }

    // cleanup logger
    delete logger_;
    logger_ = nullptr;
}