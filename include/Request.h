#pragma once
#include <string>

/**
 * @brief Represents a single web request handled by the load balancer.
 */
struct Request {
    std::string ip_in;      ///< requester IP address
    std::string ip_out;     ///< destination/result IP address
    int time_required;      ///< processing time in clock cycles
    char job_type;          ///< 'P' (processing) or 'S' (streaming)
};