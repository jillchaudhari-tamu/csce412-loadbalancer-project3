#include "WebServer.h"

WebServer::WebServer(int id)
    : id_(id), busy_(false), remaining_(0), current_(std::nullopt) {}

void WebServer::assign(const Request& r) {
    // UML: void return, assumes caller checks isIdle()
    current_ = r;
    remaining_ = r.time_required;
    busy_ = true;
}

void WebServer::tick() {
    if (!busy_) return;

    if (remaining_ > 0) remaining_--;

    if (remaining_ <= 0) {
        busy_ = false;
        remaining_ = 0;
        current_.reset();
    }
}

bool WebServer::isIdle() const {
    return !busy_;
}