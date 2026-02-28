#include "Simulation.h"
#include <iostream>

Simulation::Simulation(const Config& cfg)
    : currentTime_(0),
      maxTime_(cfg.totalCycles),
      cfg_(cfg),
      lb_(cfg_, "MAIN", "logs/loadbalancer_log.txt") {}

void Simulation::runSimulation() {
    std::cout << "\n=== LoadBalancer run start ===\n";
    std::cout << "Servers: " << cfg_.numServers << "\n";
    std::cout << "Total cycles: " << cfg_.totalCycles << "\n";

    for (currentTime_ = 0; currentTime_ < maxTime_; currentTime_++) {
        lb_.dispatch();
        lb_.scaleServers();

        if (cfg_.logCheckpointInterval > 0 && (currentTime_ % cfg_.logCheckpointInterval == 0)) {
            std::cout << "Cycle " << currentTime_ << " checkpoint\n";
        }
    }

    lb_.generateSummary();
    std::cout << "=== LoadBalancer run end ===\n\n";
}