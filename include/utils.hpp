#ifndef UTILS_HPP
#define UTILS_HPP

#include <string>
#include <vector>
#include <string_view>
#include <chrono>
#include <concepts>
#include <optional>
#include "Task.hpp"  // For TaskStatus and TaskPriority enums

namespace Utils {
    // String utilities with modern C++20 concepts
    template<typename T>
    concept StringLike = std::convertible_to<T, std::string_view>;

    [[nodiscard]] std::string trim(std::string_view str);
    [[nodiscard]] std::vector<std::string> split(std::string_view str, char delimiter);
    [[nodiscard]] std::string toLowerCase(std::string_view str);
    [[nodiscard]] std::string toUpperCase(std::string_view str);
    [[nodiscard]] bool startsWith(std::string_view str, std::string_view prefix);
    [[nodiscard]] bool endsWith(std::string_view str, std::string_view suffix);
    [[nodiscard]] bool contains(std::string_view str, std::string_view substring) noexcept;

    // Enhanced validation utilities
    [[nodiscard]] bool isValidStatus(int status) noexcept;
    [[nodiscard]] bool isValidPriority(int priority) noexcept;
    [[nodiscard]] bool isNumber(std::string_view str) noexcept;
    [[nodiscard]] bool isValidDate(std::string_view date_str);
    [[nodiscard]] bool isValidTaskName(std::string_view name);

    // Date/time utilities
    [[nodiscard]] std::string formatDateTime(const std::chrono::system_clock::time_point& tp);
    [[nodiscard]] std::string formatDate(const std::chrono::system_clock::time_point& tp);
    [[nodiscard]] std::string formatTimeAgo(const std::chrono::system_clock::time_point& tp);
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> parseDate(std::string_view date_str);

    // Enhanced flexible date parsing
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> parseFlexibleDate(std::string_view date_str);
    [[nodiscard]] std::chrono::system_clock::time_point getToday();
    [[nodiscard]] std::chrono::system_clock::time_point getTomorrow();
    [[nodiscard]] std::chrono::system_clock::time_point getYesterday();
    [[nodiscard]] std::chrono::system_clock::time_point getNextWeek();
    [[nodiscard]] std::chrono::system_clock::time_point getNextMonth();
    [[nodiscard]] std::chrono::system_clock::time_point addDays(const std::chrono::system_clock::time_point& tp, int days);
    [[nodiscard]] std::chrono::system_clock::time_point addWeeks(const std::chrono::system_clock::time_point& tp, int weeks);
    [[nodiscard]] std::string getRelativeDateExamples();

    // Enhanced display utilities
    void printHeader();
    void printSubHeader(std::string_view title);
    void printVersion();
    void printHelp();
    void printAdvancedHelp();
    void printSeparator(char ch = '-', int width = 60);
    void printSuccess(std::string_view message);
    void printError(std::string_view message);
    void printWarning(std::string_view message);
    void printInfo(std::string_view message);

    // Progress and statistics
    void printProgressBar(double percentage, int width = 30);
    void printTaskStatistics(size_t total, size_t todo, size_t in_progress, size_t completed);

    // Input utilities
    [[nodiscard]] std::string getInput(std::string_view prompt);
    [[nodiscard]] bool confirmAction(std::string_view message);
    [[nodiscard]] int getIntInput(std::string_view prompt, int min_val, int max_val);

    // File utilities
    [[nodiscard]] bool fileExists(std::string_view filepath);
    [[nodiscard]] std::string getFileSize(std::string_view filepath);
    [[nodiscard]] std::string getBackupFilename(std::string_view original_filename);

    // Color constants for terminal output
    inline constexpr std::string_view RESET = "\033[0m";
    inline constexpr std::string_view RED = "\033[31m";
    inline constexpr std::string_view GREEN = "\033[32m";
    inline constexpr std::string_view YELLOW = "\033[33m";
    inline constexpr std::string_view BLUE = "\033[34m";
    inline constexpr std::string_view MAGENTA = "\033[35m";
    inline constexpr std::string_view CYAN = "\033[36m";
    inline constexpr std::string_view WHITE = "\033[37m";
    inline constexpr std::string_view BOLD = "\033[1m";
    inline constexpr std::string_view DIM = "\033[2m";
    inline constexpr std::string_view UNDERLINE = "\033[4m";

    // Background colors
    inline constexpr std::string_view BG_RED = "\033[41m";
    inline constexpr std::string_view BG_GREEN = "\033[42m";
    inline constexpr std::string_view BG_YELLOW = "\033[43m";
    inline constexpr std::string_view BG_BLUE = "\033[44m";

    // Status and priority colors
    [[nodiscard]] std::string_view getStatusColor(TaskStatus status);
    [[nodiscard]] std::string_view getPriorityColor(TaskPriority priority);

    // Status and priority string conversions
    [[nodiscard]] std::string getStatusString(TaskStatus status);
    [[nodiscard]] std::string getPriorityString(TaskPriority priority);

    // Status and priority parsing
    [[nodiscard]] TaskStatus parseTaskStatus(std::string_view statusStr);
    [[nodiscard]] TaskPriority parseTaskPriority(std::string_view priorityStr);
}

#endif // UTILS_HPP
