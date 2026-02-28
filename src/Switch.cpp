#include "Switch.h"
#include <random>
#include <iostream>

Switch::Switch(const Config& cfg, LoadBalancer& streamingLB, LoadBalancer& processingLB)
    : cfg_(cfg), stream_(streamingLB), proc_(processingLB), factory_(cfg) {}

void Switch::route(const Request& r) {
    if (r.job_type == 'S') {
        stream_.addRequest(r);
        routedStream_++;
    } else {
        proc_.addRequest(r);
        routedProc_++;
    }
}

void Switch::maybeGenerateAndRoute() {
    static std::mt19937 rng(std::random_device{}());
    std::bernoulli_distribution coin(cfg_.newRequestProb);

    if (coin(rng)) {
        Request r = factory_.makeRequest();   // produces job_type 'S'/'P'
        route(r);
    }
}

void Switch::step() {
    time_++;

    maybeGenerateAndRoute();

    // tick both load balancers each cycle
    stream_.dispatch();
    stream_.scaleServers();

    proc_.dispatch();
    proc_.scaleServers();
}

void Switch::summary() {
    std::cout << "\n=== Switch Summary ===\n";
    std::cout << "Routed to streaming:  " << routedStream_ << "\n";
    std::cout << "Routed to processing: " << routedProc_ << "\n";
    std::cout << "Streaming LB: queue=" << stream_.queueSize()
              << " servers=" << stream_.serverCount()
              << " processed=" << stream_.processed()
              << " dropped=" << stream_.dropped() << "\n";
    std::cout << "Processing LB: queue=" << proc_.queueSize()
              << " servers=" << proc_.serverCount()
              << " processed=" << proc_.processed()
              << " dropped=" << proc_.dropped() << "\n";
    std::cout << "======================\n\n";

    stream_.generateSummary();
    proc_.generateSummary();
}