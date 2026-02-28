#pragma once
#include <string>

/**
 * @class Request
 * @brief Represents a single web request handled by the load balancer.
 */
struct Request {
    std::string ip_in;      ///< requester IP address
    std::string ip_out;     ///< destination/result IP address
    int time_required;      ///< processing time in clock cycles
    char job_type;          ///< 'P' (processing) or 'S' (streaming)

    /**
     * @brief Convert the request to a readable string (for logging/debug).
     */
    std::string toString() const {
        return ip_in + " -> " + ip_out +
               " | time=" + std::to_string(time_required) +
               " | type=" + std::string(1, job_type);
    }
};