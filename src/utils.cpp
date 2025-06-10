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
        return std::string{ str.substr(start, end - start + 1) };
    }

    std::vector<std::string> split(std::string_view str, char delimiter) {
        std::vector<std::string> tokens;
        tokens.reserve(8); // Reserve space for typical usage - Phase 1 optimization
        std::stringstream ss{ std::string{str} };
        std::string token;

        while (std::getline(ss, token, delimiter)) {
            if (auto trimmed = trim(token); !trimmed.empty()) {
                tokens.push_back(std::move(trimmed));
            }
        }

        return tokens;
    }

    std::string toLowerCase(std::string_view str) {
        std::string result;
        result.reserve(str.size());  // Avoid reallocations - Phase 1 optimization
        std::ranges::transform(str, std::back_inserter(result),
            [](unsigned char c) { return std::tolower(c); });
        return result;
    }

    bool startsWith(std::string_view str, std::string_view prefix) {
        return str.size() >= prefix.size() && str.substr(0, prefix.size()) == prefix;
    }

    bool endsWith(std::string_view str, std::string_view suffix) {
        return str.size() >= suffix.size() && str.substr(str.size() - suffix.size()) == suffix;
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

    // Date parsing helper functions
    std::optional<system_clock::time_point> tryParseStandardFormat(std::string_view dateStr, const char* format) {
        std::istringstream iss{ std::string{dateStr} };
        std::tm tm = {};
        if (iss >> std::get_time(&tm, format)) {
            return system_clock::from_time_t(std::mktime(&tm));
        }
        return std::nullopt;
    }

    std::optional<system_clock::time_point> parseDate(std::string_view dateStr) {
        // First try flexible date parsing
        if (auto result = parseFlexibleDate(dateStr)) {
            return result;
        }

        // Try standard date formats in order of preference
        const char* formats[] = { "%Y-%m-%d", "%d/%m/%Y", "%m/%d/%Y" };

        for (const auto& format : formats) {
            if (auto result = tryParseStandardFormat(dateStr, format)) {
                return result;
            }
        }

        return std::nullopt;
    }

    // Enhanced flexible date parsing for natural language
    std::optional<system_clock::time_point> parseFlexibleDate(std::string_view dateStr) {
        std::string lowercaseDate = toLowerCase(dateStr);

        // Handle "today"
        if (lowercaseDate == "today") {
            return getToday();
        }

        // Handle "tomorrow"
        if (lowercaseDate == "tomorrow") {
            return getTomorrow();
        }

        // Handle "after X days" pattern
        if (startsWith(lowercaseDate, "after ") && endsWith(lowercaseDate, " days")) {
            std::string numberPart = lowercaseDate.substr(6); // Remove "after "
            size_t spacePos = numberPart.find(' ');
            if (spacePos != std::string::npos) {
                std::string numStr = numberPart.substr(0, spacePos);
                if (isNumber(numStr)) {
                    int days = std::stoi(numStr);
                    if (days > 0) { // Only allow positive numbers
                        return addDays(system_clock::now(), days);
                    }
                }
            }
        }

        return std::nullopt; // Could not parse
    }

    // Helper functions for date calculations
    std::chrono::system_clock::time_point getToday() {
        auto now = system_clock::now();
        auto time_t_now = system_clock::to_time_t(now);
        auto tm_now = *std::localtime(&time_t_now);

        // Set to start of day (midnight)
        tm_now.tm_hour = 0;
        tm_now.tm_min = 0;
        tm_now.tm_sec = 0;

        return system_clock::from_time_t(std::mktime(&tm_now));
    }

    std::chrono::system_clock::time_point getTomorrow() {
        return addDays(getToday(), 1);
    }

    std::chrono::system_clock::time_point getYesterday() {
        return addDays(getToday(), -1);
    }

    std::chrono::system_clock::time_point getNextWeek() {
        return addDays(getToday(), 7);
    }

    // Helper to calculate days in a month
    int getDaysInMonth(int month, int year) {
        const int daysPerMonth[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
        if (month == 1 && ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0)) {
            return 29; // Leap year February
        }
        return daysPerMonth[month];
    }

    std::chrono::system_clock::time_point getNextMonth() {
        auto today = getToday();
        auto time_t_today = system_clock::to_time_t(today);
        auto tm_today = *std::localtime(&time_t_today);

        // Add one month
        tm_today.tm_mon += 1;
        if (tm_today.tm_mon > 11) {
            tm_today.tm_mon = 0;
            tm_today.tm_year += 1;
        }

        // Handle day overflow (e.g., Jan 31 + 1 month = Feb 28/29)
        int maxDay = getDaysInMonth(tm_today.tm_mon, tm_today.tm_year + 1900);
        if (tm_today.tm_mday > maxDay) {
            tm_today.tm_mday = maxDay;
        }

        return system_clock::from_time_t(std::mktime(&tm_today));
    }

    std::chrono::system_clock::time_point addDays(const std::chrono::system_clock::time_point& tp, int days) {
        return tp + std::chrono::hours(24 * days);
    }

    std::chrono::system_clock::time_point addWeeks(const std::chrono::system_clock::time_point& tp, int weeks) {
        return addDays(tp, weeks * 7);
    }

    std::string getRelativeDateExamples() {
        return R"(Supported date formats:
  • Absolute: 2024-12-25, 25/12/2024, 12/25/2024
  • Simple: today, tomorrow
  • Relative: after 3 days, after 10 days, after 30 days)";
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
        std::cout << "    - Set due date for a task" << std::endl;
        std::cout << "    - Supports flexible date formats:" << std::endl;
        std::cout << "      • Absolute: 2024-12-25, 25/12/2024, 12/25/2024" << std::endl;
        std::cout << "      • Simple: today, tomorrow" << std::endl;
        std::cout << "      • Relative: after 3 days, after 10 days" << std::endl;
        std::cout << "    - Example: due 1 \"after 5 days\"" << std::endl << std::endl;

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

    void printAdvancedHelp() {
        std::cout << BOLD << "SIMPLIFIED DATE FORMATS - DETAILED GUIDE" << RESET << std::endl;
        std::cout << "=========================================" << std::endl << std::endl;

        std::cout << BOLD << "ABSOLUTE DATE FORMATS:" << RESET << std::endl;
        std::cout << "  • " << GREEN << "2024-12-25" << RESET << " - ISO format (YYYY-MM-DD)" << std::endl;
        std::cout << "  • " << GREEN << "25/12/2024" << RESET << " - European format (DD/MM/YYYY)" << std::endl;
        std::cout << "  • " << GREEN << "12/25/2024" << RESET << " - American format (MM/DD/YYYY)" << std::endl << std::endl;

        std::cout << BOLD << "SIMPLE RELATIVE FORMATS:" << RESET << std::endl;
        std::cout << "  • " << GREEN << "today" << RESET << " - Current date" << std::endl;
        std::cout << "  • " << GREEN << "tomorrow" << RESET << " - Next day" << std::endl << std::endl;

        std::cout << BOLD << "AFTER X DAYS FORMAT:" << RESET << std::endl;
        std::cout << "  • " << GREEN << "after 1 days" << RESET << " - Same as tomorrow" << std::endl;
        std::cout << "  • " << GREEN << "after 3 days" << RESET << " - 3 days from today" << std::endl;
        std::cout << "  • " << GREEN << "after 7 days" << RESET << " - 7 days from today" << std::endl;
        std::cout << "  • " << GREEN << "after 30 days" << RESET << " - 30 days from today" << std::endl << std::endl;

        std::cout << BOLD << "EXAMPLES:" << RESET << std::endl;
        std::cout << "  " << CYAN << "./todo due 1 \"today\"" << RESET << std::endl;
        std::cout << "  " << CYAN << "./todo due 2 \"tomorrow\"" << RESET << std::endl;
        std::cout << "  " << CYAN << "./todo due 3 \"after 5 days\"" << RESET << std::endl;
        std::cout << "  " << CYAN << "./todo due 4 \"after 14 days\"" << RESET << std::endl;
        std::cout << "  " << CYAN << "./todo due 5 \"2024-12-25\"" << RESET << std::endl << std::endl;

        std::cout << YELLOW << "Note: All formats are case-insensitive" << RESET << std::endl;
    }

    void printSeparator(char ch, int width) {
        std::cout << std::string(width, ch) << std::endl;
    }

    // Input utilities
    bool confirmAction(std::string_view message) {
        std::cout << YELLOW << message << " (y/N): " << RESET;
        std::string response;
        std::getline(std::cin, response);

        // Convert to lowercase for comparison
        std::string lower_response = toLowerCase(response);
        return lower_response == "y" || lower_response == "yes";
    }

}
