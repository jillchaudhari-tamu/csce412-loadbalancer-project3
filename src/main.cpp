#include <iostream>
#include <algorithm>
#include "Config.h"
#include "Simulation.h"
#include "ConfigLoader.h"
#include "LoadBalancer.h"
#include "Switch.h"

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

    int mode;
    std::cout << "Mode (1 = single LB, 2 = switch bonus): ";
    std::cin >> mode;

    if (mode == 1) {
        // ===== Single Load Balancer Mode =====
        Simulation sim(cfg);
        sim.runSimulation();
    }
    else {
        // ===== Switch Bonus Mode =====

        Config streamCfg = cfg;
        Config procCfg   = cfg;

        int streamServers = std::max(1, cfg.numServers / 2);
        int procServers   = std::max(1, cfg.numServers - streamServers);

        streamCfg.numServers = streamServers;
        procCfg.numServers   = procServers;

        LoadBalancer streamLB(streamCfg, "STREAM", "logs/stream_lb.txt",
                            false, false);

        LoadBalancer procLB(procCfg, "PROC", "logs/proc_lb.txt",
                            false, false);

        Switch sw(cfg, streamLB, procLB);

        RequestFactory rf(cfg);

        for (int i = 0; i < cfg.numServers * cfg.initialQueueMultiplier; i++) {
            sw.route(rf.makeRequest());
        }

        for (int t = 0; t < cfg.totalCycles; t++) {
            sw.step();
        }

        sw.summary();
    }
}