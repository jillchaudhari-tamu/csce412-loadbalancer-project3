#pragma once

/**
 * @brief Holds all configurable parameters for the load balancer simulation.
 */
struct Config {
    int numServers = 10;              // user input
    int totalCycles = 10000;          // user input

    int initialQueueMultiplier = 100; // "usually servers * 100" per spec
    int minQueuePerServer = 50;       // lower threshold
    int maxQueuePerServer = 80;       // upper threshold

    int scaleCooldownN = 50;          // "wait n clock cycles" (you can tune later)

    int taskTimeMin = 10;             // range for task times (must be logged later)
    int taskTimeMax = 100;

    double newRequestProb = 0.30;     // chance each cycle to generate a new request (tune later)
};