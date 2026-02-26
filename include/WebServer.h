#pragma once
#include <optional>
#include "Request.h"

/**
 * @brief Simulates a web server that processes one request at a time.
 */
class WebServer {
public:
    explicit WebServer(int id);

    // Assign a request if the server is idle. Returns true if accepted.
    bool assign(const Request& r);

    // Process one clock cycle (decrement remaining time if busy).
    void tick();

    // Status helpers
    bool isBusy() const;
    int id() const;

    // For logging/debug
    int remainingTime() const;
    std::optional<Request> currentRequest() const;

private:
    int id_;
    bool busy_;
    int remaining_;
    std::optional<Request> current_;
};