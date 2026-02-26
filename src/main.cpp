#include <iostream>
#include "Config.h"
#include "LoadBalancer.h"

int main() {
    Config cfg;

    // ---- User Input ----
    std::cout << "Enter number of initial servers: ";
    std::cin >> cfg.numServers;

    std::cout << "Enter number of clock cycles to run: ";
    std::cin >> cfg.totalCycles;

    // ---- Basic Safety Checks ----
    if (cfg.numServers < 1) cfg.numServers = 1;
    if (cfg.totalCycles < 1) cfg.totalCycles = 1;
    if (cfg.taskTimeMin < 1) cfg.taskTimeMin = 1;
    if (cfg.taskTimeMax < cfg.taskTimeMin)
        cfg.taskTimeMax = cfg.taskTimeMin;

    int initialQueueSize = cfg.numServers * cfg.initialQueueMultiplier;

    // ---- Configuration Summary ----
    std::cout << "\n=== Simulation Configuration ===\n";
    std::cout << "Servers: " << cfg.numServers << "\n";
    std::cout << "Total cycles: " << cfg.totalCycles << "\n";
    std::cout << "Initial queue size: " << initialQueueSize
              << " (servers * " << cfg.initialQueueMultiplier << ")\n";
    std::cout << "Task time range: [" << cfg.taskTimeMin
              << ", " << cfg.taskTimeMax << "]\n";
    std::cout << "Queue thresholds: "
              << (cfg.minQueuePerServer * cfg.numServers)
              << " to "
              << (cfg.maxQueuePerServer * cfg.numServers)
              << "\n";
    std::cout << "Scale cooldown (n): "
              << cfg.scaleCooldownN << " cycles\n";
    std::cout << "New request probability/cycle: "
              << cfg.newRequestProb << "\n";
    std::cout << "===============================\n\n";

    // ---- Run Simulation ----
    LoadBalancer lb(cfg);
    lb.run();

    return 0;
}