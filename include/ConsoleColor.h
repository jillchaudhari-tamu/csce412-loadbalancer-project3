#pragma once
#include <string>

namespace ConsoleColor {
    static const std::string RESET  = "\033[0m";
    static const std::string RED    = "\033[31m";
    static const std::string GREEN  = "\033[32m";
    static const std::string YELLOW = "\033[33m";
    static const std::string CYAN   = "\033[36m";

    inline std::string wrap(bool enabled, const std::string& color, const std::string& text) {
        if (!enabled) return text;
        return color + text + RESET;
    }
}