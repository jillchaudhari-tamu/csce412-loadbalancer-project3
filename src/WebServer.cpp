#include "WebServer.h"

WebServer::WebServer(int id)
    : id_(id), busy_(false), remaining_(0), current_(std::nullopt) {}

bool WebServer::assign(const Request& r) {
    if (busy_) return false;
    current_ = r;
    remaining_ = r.time_required;
    busy_ = true;
    return true;
}

void WebServer::tick() {
    if (!busy_) return;

    if (remaining_ > 0) remaining_--;

    // finished
    if (remaining_ <= 0) {
        busy_ = false;
        current_.reset();
        remaining_ = 0;
    }
}

bool WebServer::isBusy() const { return busy_; }
int WebServer::id() const { return id_; }
int WebServer::remainingTime() const { return remaining_; }
std::optional<Request> WebServer::currentRequest() const { return current_; }