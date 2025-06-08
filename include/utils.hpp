#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>

namespace Utils {
    // String utilities
    std::string trim(const std::string& str);
    std::vector<std::string> split(const std::string& str, char delimiter);
    std::string toLowerCase(const std::string& str);
    
    // Validation utilities
    bool isValidStatus(int status);
    bool isValidPriority(int priority);
    bool isNumber(const std::string& str);
    
    // Display utilities
    void printHeader();
    void printVersion();
    void printHelp();
    void printSeparator();
    
    // Color constants for terminal output
    const std::string RESET = "\033[0m";
    const std::string RED = "\033[31m";
    const std::string GREEN = "\033[32m";
    const std::string YELLOW = "\033[33m";
    const std::string BLUE = "\033[34m";
    const std::string MAGENTA = "\033[35m";
    const std::string CYAN = "\033[36m";
    const std::string BOLD = "\033[1m";
}

#endif // UTILS_HPP
