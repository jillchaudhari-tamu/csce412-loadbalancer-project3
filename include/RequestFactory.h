#pragma once
#include <random>
#include <string>
#include "Config.h"
#include "Request.h"

/**
 * @brief Generates random requests (IPs, times, and job types).
 */
class RequestFactory {
public:
    explicit RequestFactory(const Config& cfg)
        : cfg_(cfg), rng_(std::random_device{}()),
          octetDist_(0, 255),
          timeDist_(cfg_.taskTimeMin, cfg_.taskTimeMax),
          jobDist_(0, 1) {}

    Request makeRequest() {
        Request r;
        r.ip_in = randomIP();
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

    std::string randomIP() {
        return std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_)) + "." +
               std::to_string(octetDist_(rng_));
    }
};