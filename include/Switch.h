#pragma once
#include <string>
#include "LoadBalancer.h"
#include "Request.h"
#include "RequestFactory.h"
#include "Config.h"

class Switch {
public:
    Switch(const Config& cfg,
           LoadBalancer& streamingLB,
           LoadBalancer& processingLB);

    void route(const Request& r);
    void step();          // one simulation cycle
    void summary();       // combined + per-LB output

private:
    const Config& cfg_;
    LoadBalancer& stream_;
    LoadBalancer& proc_;
    RequestFactory factory_;

    long long routedStream_ = 0;
    long long routedProc_ = 0;

    int time_ = 0;

    void maybeGenerateAndRoute(); // uses cfg_.newRequestProb
};