#pragma once
#include <fstream>
#include <string>

/**
 * @class Logger
 * @brief Writes simulation events and a final summary to a log file.
 */
class Logger {
public:
    explicit Logger(const std::string& filename);
    ~Logger();

    void logLine(const std::string& line);

private:
    std::ofstream out_;
};