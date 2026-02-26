#pragma once
#include "Config.h"
#include "LoadBalancer.h"

/**
 * @class Simulation
 * @brief Top-level driver that runs the load balancer for a fixed number of clock cycles.
 */
class Simulation {
public:
    explicit Simulation(const Config& cfg);

    // --- UML method ---
    void runSimulation();

private:
    int currentTime_;
    int maxTime_;
    Config cfg_;
    LoadBalancer lb_;
};