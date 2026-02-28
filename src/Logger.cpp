#include "Logger.h"

Logger::Logger(const std::string& filename)
    : out_(filename) {}

Logger::~Logger() {
    if (out_.is_open()) out_.close();
}

void Logger::logLine(const std::string& line) {
    if (out_.is_open()) out_ << line << "\n";
}