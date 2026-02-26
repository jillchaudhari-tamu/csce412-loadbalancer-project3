#include "Simulation.h"
#include <iostream>

Simulation::Simulation(const Config& cfg)
    : currentTime_(0),
      maxTime_(cfg.totalCycles),
      cfg_(cfg),
      lb_(cfg_) {}

void Simulation::runSimulation() {
    std::cout << "\n=== LoadBalancer run start ===\n";
    std::cout << "Servers: " << cfg_.numServers << "\n";
    std::cout << "Total cycles: " << cfg_.totalCycles << "\n";

    for (currentTime_ = 0; currentTime_ < maxTime_; currentTime_++) {
        // Per-cycle workflow (matches your flowchart):
        lb_.dispatch();
        // For now, just scale and print; we’ll shift tickServers into dispatch() in the next micro-step.
        lb_.scaleServers();

        if (currentTime_ % 1000 == 0) {
            // lightweight status; we’ll make this nicer when logging to file
            std::cout << "Cycle " << currentTime_ << " checkpoint\n";
        }
    }

    lb_.generateSummary();
    std::cout << "=== LoadBalancer run end ===\n\n";
}