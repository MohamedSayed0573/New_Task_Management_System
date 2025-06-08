#include "utils.hpp"
#include <iostream>
#include <algorithm>
#include <sstream>
#include <cctype>
#include <iomanip>
#include <chrono>
#include <format>

using namespace std::chrono;

namespace Utils {

// String utilities
std::string trim(std::string_view str) {
    if (str.empty()) return "";
    
    size_t start = str.find_first_not_of(" \t\n\r");
    if (start == std::string_view::npos) return "";
    
    size_t end = str.find_last_not_of(" \t\n\r");
    return std::string{str.substr(start, end - start + 1)};
}

std::vector<std::string> split(std::string_view str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss{std::string{str}};
    std::string token;
    
    while (std::getline(ss, token, delimiter)) {
        if (auto trimmed = trim(token); !trimmed.empty()) {
            tokens.push_back(std::move(trimmed));
        }
    }
    
    return tokens;
}

std::string toLowerCase(std::string_view str) {
    std::string result{str};
    std::ranges::transform(result, result.begin(), 
                          [](unsigned char c) { return std::tolower(c); });
    return result;
}

bool contains(std::string_view haystack, std::string_view needle) noexcept {
    return haystack.find(needle) != std::string_view::npos;
}

bool isNumber(std::string_view str) noexcept {
    if (str.empty()) return false;
    
    return std::ranges::all_of(str, [](char c) {
        return std::isdigit(static_cast<unsigned char>(c));
    });
}

// Date/time utilities
std::string formatDateTime(const system_clock::time_point& timePoint) {
    auto time_t_val = system_clock::to_time_t(timePoint);
    auto tm_val = *std::localtime(&time_t_val);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

std::string formatDate(const system_clock::time_point& timePoint) {
    auto time_t_val = system_clock::to_time_t(timePoint);
    auto tm_val = *std::localtime(&time_t_val);
    
    std::ostringstream oss;
    oss << std::put_time(&tm_val, "%Y-%m-%d");
    return oss.str();
}

std::optional<system_clock::time_point> parseDate(std::string_view dateStr) {
    std::istringstream iss{std::string{dateStr}};
    std::tm tm = {};
    
    // Try YYYY-MM-DD format
    if (iss >> std::get_time(&tm, "%Y-%m-%d")) {
        return system_clock::from_time_t(std::mktime(&tm));
    }
    
    // Try DD/MM/YYYY format
    iss.clear();
    iss.str(std::string{dateStr});
    if (iss >> std::get_time(&tm, "%d/%m/%Y")) {
        return system_clock::from_time_t(std::mktime(&tm));
    }
    
    return std::nullopt;
}

// Status and priority utilities
std::string getStatusString(TaskStatus status) {
    switch (status) {
        case TaskStatus::TODO: return "To-Do";
        case TaskStatus::IN_PROGRESS: return "In Progress";
        case TaskStatus::COMPLETED: return "Completed";
        default: return "Unknown";
    }
}

std::string getPriorityString(TaskPriority priority) {
    switch (priority) {
        case TaskPriority::LOW: return "Low";
        case TaskPriority::MEDIUM: return "Medium";
        case TaskPriority::HIGH: return "High";
        default: return "Unknown";
    }
}

std::string_view getStatusColor(TaskStatus status) {
    switch (status) {
        case TaskStatus::TODO: return RED;
        case TaskStatus::IN_PROGRESS: return YELLOW;
        case TaskStatus::COMPLETED: return GREEN;
        default: return RESET;
    }
}

std::string_view getPriorityColor(TaskPriority priority) {
    switch (priority) {
        case TaskPriority::LOW: return BLUE;
        case TaskPriority::MEDIUM: return YELLOW;
        case TaskPriority::HIGH: return RED;
        default: return RESET;
    }
}

TaskStatus parseTaskStatus(std::string_view statusStr) {
    std::string lower = toLowerCase(statusStr);
    
    if (lower == "todo" || lower == "1") return TaskStatus::TODO;
    if (lower == "inprogress" || lower == "in-progress" || lower == "2") return TaskStatus::IN_PROGRESS;
    if (lower == "completed" || lower == "done" || lower == "3") return TaskStatus::COMPLETED;
    
    throw std::invalid_argument(std::format("Invalid status: {}", statusStr));
}

TaskPriority parseTaskPriority(std::string_view priorityStr) {
    std::string lower = toLowerCase(priorityStr);
    
    if (lower == "low" || lower == "1") return TaskPriority::LOW;
    if (lower == "medium" || lower == "2") return TaskPriority::MEDIUM;
    if (lower == "high" || lower == "3") return TaskPriority::HIGH;
    
    throw std::invalid_argument(std::format("Invalid priority: {}", priorityStr));
}

// Display utilities
void printHeader() {
    std::cout << BOLD << CYAN;
    std::cout << "╔════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║                    TO-DO LIST MANAGER                  ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════╝" << RESET << std::endl;
}

void printVersion() {
    std::cout << BOLD << GREEN;
    std::cout << "To-Do List Manager v2.0.0" << std::endl;
    std::cout << "Built with C++20/23 - Enhanced Edition" << RESET << std::endl;
}

void printHelp() {
    std::cout << BOLD << "TO-DO LIST MANAGER - HELP" << RESET << std::endl;
    std::cout << "=========================" << std::endl << std::endl;
    
    std::cout << BOLD << "BASIC COMMANDS:" << RESET << std::endl;
    std::cout << GREEN << "  add <task_name> [status] [priority]" << RESET << std::endl;
    std::cout << "    - Add a new task" << std::endl;
    std::cout << "    - Status: todo|inprogress|completed (default: todo)" << std::endl;
    std::cout << "    - Priority: low|medium|high (default: low)" << std::endl;
    std::cout << "    - Example: add \"Learn C++\" inprogress high" << std::endl << std::endl;
    
    std::cout << GREEN << "  show [filter]" << RESET << std::endl;
    std::cout << "    - Show all tasks or filtered tasks" << std::endl;
    std::cout << "    - Filters: todo, inprogress, completed, low, medium, high" << std::endl;
    std::cout << "    - Example: show todo" << std::endl << std::endl;
    
    std::cout << GREEN << "  update <task_ID> <name> <status> <priority>" << RESET << std::endl;
    std::cout << "    - Update an existing task (all parameters required)" << std::endl;
    std::cout << "    - Example: update 1 \"Updated task\" completed high" << std::endl << std::endl;
    
    std::cout << GREEN << "  remove/rm <task_ID>" << RESET << std::endl;
    std::cout << "    - Remove a task by ID" << std::endl;
    std::cout << "    - Example: remove 1" << std::endl << std::endl;
    
    std::cout << GREEN << "  search <query>" << RESET << std::endl;
    std::cout << "    - Search tasks by name, description, or tags" << std::endl;
    std::cout << "    - Example: search \"Learn\"" << std::endl << std::endl;
    
    std::cout << BOLD << "ADVANCED COMMANDS:" << RESET << std::endl;
    std::cout << GREEN << "  detail <task_ID>" << RESET << std::endl;
    std::cout << "    - Show detailed information about a task" << std::endl << std::endl;
    
    std::cout << GREEN << "  overdue" << RESET << std::endl;
    std::cout << "    - Show all overdue tasks" << std::endl << std::endl;
    
    std::cout << GREEN << "  stats" << RESET << std::endl;
    std::cout << "    - Show task statistics and summary" << std::endl << std::endl;
    
    std::cout << GREEN << "  tag <task_ID> <tag>" << RESET << std::endl;
    std::cout << "    - Add a tag to a task" << std::endl << std::endl;
    
    std::cout << GREEN << "  untag <task_ID> <tag>" << RESET << std::endl;
    std::cout << "    - Remove a tag from a task" << std::endl << std::endl;
    
    std::cout << GREEN << "  due <task_ID> <date>" << RESET << std::endl;
    std::cout << "    - Set due date for a task (YYYY-MM-DD format)" << std::endl << std::endl;
    
    std::cout << GREEN << "  complete <task_ID>" << RESET << std::endl;
    std::cout << "    - Mark a task as completed" << std::endl << std::endl;
    
    std::cout << GREEN << "  --help/-h" << RESET << std::endl;
    std::cout << "    - Show this help menu" << std::endl << std::endl;
    
    std::cout << GREEN << "  --version/-v" << RESET << std::endl;
    std::cout << "    - Show version information" << std::endl << std::endl;
    
    std::cout << BOLD << "STATUS VALUES:" << RESET << std::endl;
    std::cout << "  " << RED << "todo" << RESET << " - Task to be done" << std::endl;
    std::cout << "  " << YELLOW << "inprogress" << RESET << " - Task in progress" << std::endl;
    std::cout << "  " << GREEN << "completed" << RESET << " - Task completed" << std::endl << std::endl;
    
    std::cout << BOLD << "PRIORITY VALUES:" << RESET << std::endl;
    std::cout << "  " << BLUE << "low" << RESET << " - Low priority" << std::endl;
    std::cout << "  " << YELLOW << "medium" << RESET << " - Medium priority" << std::endl;
    std::cout << "  " << RED << "high" << RESET << " - High priority" << std::endl;
}

void printSeparator(char ch, int width) {
    std::cout << std::string(width, ch) << std::endl;
}

}
