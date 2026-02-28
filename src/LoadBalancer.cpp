#include "LoadBalancer.h"
#include <iostream>
#include <random>
#include <string>
#include "ConsoleColor.h"

// helper: count busy vs idle servers (for logging checkpoints/summary)
static void countServerStates(const std::vector<WebServer>& servers, int& busy, int& idle) {
    busy = 0;
    idle = 0;
    for (const auto& s : servers) {
        if (s.isIdle()) idle++;
        else busy++;
    }
}

// -------------------- constructor --------------------

LoadBalancer::LoadBalancer(const Config& cfg,
                           std::string name,
                           std::string logFile,
                           bool doFillInitialQueue,
                           bool internalArrivals)
    : cfg_(cfg),
      factory_(cfg_),
      name_(std::move(name)),
      internalArrivals_(internalArrivals) {

    initServers();

    if (doFillInitialQueue) {
        fillInitialQueue();
    }

    startingQueueSize_ = (int)q_.size();
    peakQueue_ = startingQueueSize_;
    peakServers_ = (int)servers_.size();

    // open log file
    logger_ = new Logger(logFile);
    logger_->logLine("=== Load Balancer Log Start (" + name_ + ") ===");
    logger_->logLine("Starting queue size: " + std::to_string(startingQueueSize_));
    logger_->logLine("Task time range: [" + std::to_string(cfg_.taskTimeMin) + ", " +
                     std::to_string(cfg_.taskTimeMax) + "]");
    logger_->logLine("Initial servers: " + std::to_string((int)servers_.size()));
    logger_->logLine("Queue thresholds: " +
                     std::to_string(cfg_.minQueuePerServer * (int)servers_.size()) + " to " +
                     std::to_string(cfg_.maxQueuePerServer * (int)servers_.size()));
    logger_->logLine("Scale cooldown (n): " + std::to_string(cfg_.scaleCooldownN));
    logger_->logLine("New request probability/cycle: " + std::to_string(cfg_.newRequestProb));
    logger_->logLine("Blocked chance percent: " + std::to_string(cfg_.blockedChancePercent));
    logger_->logLine("Checkpoint interval: " + std::to_string(cfg_.logCheckpointInterval));
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

    // 172.16.0.0 - 172.31.255.255
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
        "[Scale Up][" + name_ + "] time=" + std::to_string(currentTime_) +
        " queue=" + std::to_string((int)q_.size()) +
        " servers=" + std::to_string((int)servers_.size())
    ) << "\n";

    if (logger_) {
        logger_->logLine("[Scale Up][" + name_ + "] time=" + std::to_string(currentTime_) +
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
        "[Scale Down][" + name_ + "] time=" + std::to_string(currentTime_) +
        " queue=" + std::to_string((int)q_.size()) +
        " servers=" + std::to_string((int)servers_.size())
    ) << "\n";

    if (logger_) {
        logger_->logLine("[Scale Down][" + name_ + "] time=" + std::to_string(currentTime_) +
                         " queue=" + std::to_string((int)q_.size()) +
                         " servers=" + std::to_string((int)servers_.size()));
    }
}

// -------------------- UML public methods --------------------

void LoadBalancer::addRequest(const Request& r) {
    // firewall / DOS prevention
    if (isBlockedIP(r.ip_in)) {
        dropped_++;

        // Console: show occasionally
        if (cfg_.logVerboseDrops && (dropped_ % 50 == 0)) {
            std::cout << ConsoleColor::wrap(
                cfg_.useColor,
                ConsoleColor::RED,
                "[Dropped][" + name_ + "] time=" + std::to_string(currentTime_) +
                " total_dropped=" + std::to_string(dropped_)
            ) << "\n";
        }

        // Log file: show occasionally
        if (logger_ && cfg_.logVerboseDrops && (dropped_ % 50 == 0)) {
            logger_->logLine("[Dropped][" + name_ + "] time=" + std::to_string(currentTime_) +
                             " total_dropped=" + std::to_string(dropped_));
        }
        return;
    }

    q_.push(r);
}

void LoadBalancer::dispatch() {
    currentTime_++;

    // 1) new request(s) arriving randomly this cycle (if enabled)
    if (internalArrivals_) {
        maybeGenerateRandomRequest();
    }

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

    // 4) checkpoint logging to make the log longer & more useful
    if (logger_ && cfg_.logCheckpointInterval > 0 &&
        (currentTime_ % cfg_.logCheckpointInterval == 0)) {

        int busy = 0, idle = 0;
        countServerStates(servers_, busy, idle);

        logger_->logLine("[Checkpoint][" + name_ + "] time=" + std::to_string(currentTime_) +
                         " queue=" + std::to_string((int)q_.size()) +
                         " servers=" + std::to_string((int)servers_.size()) +
                         " busy=" + std::to_string(busy) +
                         " idle=" + std::to_string(idle) +
                         " processed=" + std::to_string(processed_) +
                         " dropped=" + std::to_string(dropped_) +
                         " generated=" + std::to_string(generatedRandom_));
    }
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

    int busy = 0, idle = 0;
    countServerStates(servers_, busy, idle);

    // console summary
    std::cout << "\n=== Summary (" << name_ << ") ===\n";
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
    std::cout << "Final servers: " << (int)servers_.size() << "\n";
    std::cout << "Busy servers: " << busy << "\n";
    std::cout << "Idle servers: " << idle << "\n";
    std::cout << "=============\n\n";

    // log summary (file)
    if (logger_) {
        logger_->logLine("=== Summary (" + name_ + ") ===");
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
        logger_->logLine("Final servers: " + std::to_string((int)servers_.size()));
        logger_->logLine("Busy servers: " + std::to_string(busy));
        logger_->logLine("Idle servers: " + std::to_string(idle));
        logger_->logLine("=== Load Balancer Log End (" + name_ + ") ===");
    }

    delete logger_;
    logger_ = nullptr;
}