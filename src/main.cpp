#include <iostream>
#include "Config.h"
#include "Simulation.h"
#include "ConfigLoader.h"

int main() {
    Config cfg;
    ConfigLoader::loadFromFile("config.txt", cfg);

    std::cout << "Enter number of initial servers: ";
    std::cin >> cfg.numServers;

    std::cout << "Enter number of clock cycles to run: ";
    std::cin >> cfg.totalCycles;

    if (cfg.numServers < 1) cfg.numServers = 1;
    if (cfg.totalCycles < 1) cfg.totalCycles = 1;

    int initialQueueSize = cfg.numServers * cfg.initialQueueMultiplier;

    std::cout << "\n=== Simulation Configuration ===\n";
    std::cout << "Servers: " << cfg.numServers << "\n";
    std::cout << "Total cycles: " << cfg.totalCycles << "\n";
    std::cout << "Initial queue size: " << initialQueueSize << " (servers * "
              << cfg.initialQueueMultiplier << ")\n";
    std::cout << "Task time range: [" << cfg.taskTimeMin << ", " << cfg.taskTimeMax << "]\n";
    std::cout << "Queue thresholds: " << (cfg.minQueuePerServer * cfg.numServers)
              << " to " << (cfg.maxQueuePerServer * cfg.numServers) << "\n";
    std::cout << "Scale cooldown (n): " << cfg.scaleCooldownN << " cycles\n";
    std::cout << "New request probability/cycle: " << cfg.newRequestProb << "\n";
    std::cout << "===============================\n\n";

    Simulation sim(cfg);
    sim.runSimulation();

    return 0;
}