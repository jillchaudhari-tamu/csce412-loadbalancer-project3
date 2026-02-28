#pragma once
#include <random>
#include <string>
#include "Config.h"
#include "Request.h"

/**
 * @class RequestFactory
 * @brief Generates random Request objects for the simulation.
 */
class RequestFactory {
public:
    explicit RequestFactory(const Config& cfg)
        : cfg_(cfg),
          rng_(std::random_device{}()),
          octetDist_(0, 255),
          timeDist_(cfg_.taskTimeMin, cfg_.taskTimeMax),
          jobDist_(0, 1),
          blockChanceDist_(1, 100) {}

    Request makeRequest() {
        Request r;

        // Generate normal random IP
        r.ip_in = randomIP();

        // 10% chance to force a blocked IP (192.168.x.x)
        if (blockChanceDist_(rng_) <= cfg_.blockedChancePercent) {
            r.ip_in = "192.168." +
                      std::to_string(octetDist_(rng_)) + "." +
                      std::to_string(octetDist_(rng_));
        }

        r.ip_out = randomIP();
        r.time_required = timeDist_(rng_);
        r.job_type = (jobDist_(rng_) == 0) ? 'P' : 'S';

        return r;
    }

private:
    const Config& cfg_;
    std::mt19937 rng_;

    std::uniform_int_distribution<int> octetDist_;
    std::uniform_int_distribution<int> timeDist_;
    std::uniform_int_distribution<int> jobDist_;
    std::uniform_int_distribution<int> blockChanceDist_;

    std::string randomIP() {
        return std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_));
    }
};