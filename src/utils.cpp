#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cctype>

namespace Utils {
    
std::string trim(const std::string& str) {
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return str.substr(start, end - start + 1);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(trim(token));
    }
    
    return tokens;
}

std::string toLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(), 
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool isValidStatus(int status) {
    return status >= 1 && status <= 3;
}

bool isValidPriority(int priority) {
    return priority >= 1 && priority <= 3;
}

bool isNumber(const std::string& str) {
    if (str.empty()) return false;
    
    for (char c : str) {
        if (!std::isdigit(c)) return false;
    }
    
    return true;
}

void printHeader() {
    std::cout << BOLD << CYAN;
    std::cout << "╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    TO-DO LIST MANAGER                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << RESET << std::endl;
}

void printVersion() {
    std::cout << BOLD << GREEN;
    std::cout << "To-Do List Manager v1.0.0" << std::endl;
    std::cout << "Built with C++23" << RESET << std::endl;
}

void printHelp() {
    std::cout << BOLD << "TO-DO LIST MANAGER - HELP" << RESET << std::endl;
    std::cout << "=========================" << std::endl << std::endl;
    
    std::cout << BOLD << "COMMANDS:" << RESET << std::endl;
    std::cout << GREEN << "  add <task_name> [status] [priority]" << RESET << std::endl;
    std::cout << "    - Add a new task" << std::endl;
    std::cout << "    - Status: 1=To-Do, 2=In Progress, 3=Completed (default: 1)" << std::endl;
    std::cout << "    - Priority: 1=Low, 2=Medium, 3=High (default: 1)" << std::endl;
    std::cout << "    - Example: add \"Learn C++\" 1 3" << std::endl << std::endl;
    
    std::cout << GREEN << "  show [filter]" << RESET << std::endl;
    std::cout << "    - Show all tasks or filtered tasks" << std::endl;
    std::cout << "    - Filters: -t (To-Do), -i (In Progress), -c (Completed)" << std::endl;
    std::cout << "    -          -l (Low), -m (Medium), -h (High)" << std::endl;
    std::cout << "    - Example: show -t" << std::endl << std::endl;
    
    std::cout << GREEN << "  update <task_ID> <name> <status> <priority>" << RESET << std::endl;
    std::cout << "    - Update an existing task (all parameters required)" << std::endl;
    std::cout << "    - Example: update 1 \"Updated task\" 2 3" << std::endl << std::endl;
    
    std::cout << GREEN << "  remove/rm <task_ID>" << RESET << std::endl;
    std::cout << "    - Remove a task by ID" << std::endl;
    std::cout << "    - Example: remove 1" << std::endl << std::endl;
    
    std::cout << GREEN << "  search <task_name>" << RESET << std::endl;
    std::cout << "    - Search tasks by name (partial match)" << std::endl;
    std::cout << "    - Example: search \"Learn\"" << std::endl << std::endl;
    
    std::cout << GREEN << "  --help/-h" << RESET << std::endl;
    std::cout << "    - Show this help menu" << std::endl << std::endl;
    
    std::cout << GREEN << "  --version/-v" << RESET << std::endl;
    std::cout << "    - Show version information" << std::endl << std::endl;
    
    std::cout << BOLD << "STATUS CODES:" << RESET << std::endl;
    std::cout << "  1 - " << RED << "To-Do" << RESET << std::endl;
    std::cout << "  2 - " << YELLOW << "In Progress" << RESET << std::endl;
    std::cout << "  3 - " << GREEN << "Completed" << RESET << std::endl << std::endl;
    
    std::cout << BOLD << "PRIORITY CODES:" << RESET << std::endl;
    std::cout << "  1 - " << BLUE << "Low" << RESET << std::endl;
    std::cout << "  2 - " << YELLOW << "Medium" << RESET << std::endl;
    std::cout << "  3 - " << RED << "High" << RESET << std::endl;
}

void printSeparator() {
    std::cout << "────────────────────────────────────────────────────────" << std::endl;
}

}
