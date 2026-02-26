#pragma once
#include <optional>
#include "Request.h"

/**
 * @class WebServer
 * @brief Processes at most one Request at a time.
 */
class WebServer {
public:
    explicit WebServer(int id);

    /**
     * @brief Assign a request to this server (server must be idle).
     */
    void assign(const Request& r);

    /**
     * @brief Process one clock cycle.
     */
    void tick();

    /**
     * @brief True if the server is not currently processing a request.
     */
    bool isIdle() const;

private:
    int id_;
    bool busy_;
    int remaining_;
    std::optional<Request> current_;
};