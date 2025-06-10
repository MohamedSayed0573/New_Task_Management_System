/**
 * @file utils.hpp
 * @brief Comprehensive utility functions and constants for the todo application
 *
 * This header provides a wide range of utility functions including:
 * - Modern C++20 string manipulation with concepts
 * - Enhanced date/time parsing and formatting
 * - Terminal color constants and display utilities
 * - Input validation and user interaction helpers
 * - File system utilities and progress indicators
 */

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
    // =============================
    // Modern C++20 Concepts for Type Safety
    // =============================

    /**
     * @concept StringLike
     * @brief Concept for types that can be converted to string_view
     *
     * Enables generic string processing functions while maintaining type safety.
     * Accepts std::string, std::string_view, const char*, etc.
     */
    template<typename T>
    concept StringLike = std::convertible_to<T, std::string_view>;

    // =================================
    // Core String Manipulation Utilities  
    // =================================

    [[nodiscard]] std::string trim(std::string_view str);                           ///< Remove leading/trailing whitespace
    [[nodiscard]] std::vector<std::string> split(std::string_view str, char delimiter); ///< Split string by delimiter
    [[nodiscard]] std::string toLowerCase(std::string_view str);                    ///< Convert string to lowercase
    [[nodiscard]] std::string toUpperCase(std::string_view str);                    ///< Convert string to uppercase
    [[nodiscard]] bool startsWith(std::string_view str, std::string_view prefix);   ///< Check if string starts with prefix
    [[nodiscard]] bool endsWith(std::string_view str, std::string_view suffix);     ///< Check if string ends with suffix
    [[nodiscard]] bool contains(std::string_view str, std::string_view substring) noexcept; ///< Check if string contains substring

    // =========================
    // Enhanced Validation Utilities
    // =========================

    [[nodiscard]] bool isValidStatus(int status) noexcept;                          ///< Validate status integer (1-3)
    [[nodiscard]] bool isValidPriority(int priority) noexcept;                      ///< Validate priority integer (1-3)
    [[nodiscard]] bool isNumber(std::string_view str) noexcept;                     ///< Check if string is numeric
    [[nodiscard]] bool isValidDate(std::string_view date_str);                      ///< Validate date string format
    [[nodiscard]] bool isValidTaskName(std::string_view name);                      ///< Validate task name (non-empty, reasonable length)

    // =========================
    // Date/Time Processing Utilities
    // =========================

    [[nodiscard]] std::string formatDateTime(const std::chrono::system_clock::time_point& tp);        ///< Format full date and time
    [[nodiscard]] std::string formatDate(const std::chrono::system_clock::time_point& tp);            ///< Format date only (YYYY-MM-DD)
    [[nodiscard]] std::string formatTimeAgo(const std::chrono::system_clock::time_point& tp);         ///< Format relative time ("2 days ago")
    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> parseDate(std::string_view date_str); ///< Parse date string to time_point

    // =============================
    // Enhanced Flexible Date Parsing
    // =============================

    [[nodiscard]] std::optional<std::chrono::system_clock::time_point> parseFlexibleDate(std::string_view date_str); ///< Parse various date formats
    [[nodiscard]] std::chrono::system_clock::time_point getToday();                     ///< Get today's date at midnight
    [[nodiscard]] std::chrono::system_clock::time_point getTomorrow();                  ///< Get tomorrow's date at midnight
    [[nodiscard]] std::chrono::system_clock::time_point getYesterday();                 ///< Get yesterday's date at midnight
    [[nodiscard]] std::chrono::system_clock::time_point getNextWeek();                  ///< Get date one week from today
    [[nodiscard]] std::chrono::system_clock::time_point getNextMonth();                 ///< Get date one month from today
    [[nodiscard]] std::chrono::system_clock::time_point addDays(const std::chrono::system_clock::time_point& tp, int days);   ///< Add days to time_point
    [[nodiscard]] std::chrono::system_clock::time_point addWeeks(const std::chrono::system_clock::time_point& tp, int weeks); ///< Add weeks to time_point
    [[nodiscard]] std::string getRelativeDateExamples();                               ///< Get examples of supported date formats

    // =================================
    // Enhanced Terminal Display Utilities
    // =================================

    void printHeader();                                                             ///< Print application header with ASCII art
    void printSubHeader(std::string_view title);                                   ///< Print section header with formatting
    void printVersion();                                                            ///< Print version information
    void printHelp();                                                               ///< Print basic help information
    void printAdvancedHelp();                                                       ///< Print detailed help with examples
    void printSeparator(char ch = '-', int width = 60);                            ///< Print horizontal separator line
    void printSuccess(std::string_view message);                                   ///< Print success message with green color
    void printError(std::string_view message);                                     ///< Print error message with red color
    void printWarning(std::string_view message);                                   ///< Print warning message with yellow color
    void printInfo(std::string_view message);                                      ///< Print info message with blue color

    // ===============================
    // Progress and Statistics Display
    // ===============================

    void printProgressBar(double percentage, int width = 30);                      ///< Display ASCII progress bar
    void printTaskStatistics(size_t total, size_t todo, size_t in_progress, size_t completed); ///< Display task count summary

    // =======================
    // User Input Utilities
    // =======================

    [[nodiscard]] std::string getInput(std::string_view prompt);                    ///< Get user input with prompt
    [[nodiscard]] bool confirmAction(std::string_view message);                     ///< Get yes/no confirmation from user
    [[nodiscard]] int getIntInput(std::string_view prompt, int min_val, int max_val); ///< Get validated integer input

    // ===================
    // File System Utilities
    // ===================

    [[nodiscard]] bool fileExists(std::string_view filepath);                       ///< Check if file exists
    [[nodiscard]] std::string getFileSize(std::string_view filepath);               ///< Get human-readable file size
    [[nodiscard]] std::string getBackupFilename(std::string_view original_filename); ///< Generate backup filename

    // ===================================
    // ANSI Color Constants for Terminal Output
    // ===================================

    // Basic colors
    inline constexpr std::string_view RESET = "\033[0m";       ///< Reset all formatting
    inline constexpr std::string_view RED = "\033[31m";        ///< Red text
    inline constexpr std::string_view GREEN = "\033[32m";      ///< Green text
    inline constexpr std::string_view YELLOW = "\033[33m";     ///< Yellow text
    inline constexpr std::string_view BLUE = "\033[34m";       ///< Blue text
    inline constexpr std::string_view MAGENTA = "\033[35m";    ///< Magenta text
    inline constexpr std::string_view CYAN = "\033[36m";       ///< Cyan text
    inline constexpr std::string_view WHITE = "\033[37m";      ///< White text

    // Text formatting
    inline constexpr std::string_view BOLD = "\033[1m";        ///< Bold text
    inline constexpr std::string_view DIM = "\033[2m";         ///< Dimmed text
    inline constexpr std::string_view UNDERLINE = "\033[4m";   ///< Underlined text
    inline constexpr std::string_view BLINK = "\033[5m";       ///< Blinking text (rarely supported)
    inline constexpr std::string_view REVERSE = "\033[7m";     ///< Reverse video (swap fg/bg)

    // Bright colors
    inline constexpr std::string_view BRIGHT_RED = "\033[91m";     ///< Bright red text
    inline constexpr std::string_view BRIGHT_GREEN = "\033[92m";   ///< Bright green text
    inline constexpr std::string_view BRIGHT_YELLOW = "\033[93m";  ///< Bright yellow text
    inline constexpr std::string_view BRIGHT_BLUE = "\033[94m";    ///< Bright blue text
    inline constexpr std::string_view BRIGHT_MAGENTA = "\033[95m"; ///< Bright magenta text
    inline constexpr std::string_view BRIGHT_CYAN = "\033[96m";    ///< Bright cyan text
    inline constexpr std::string_view BRIGHT_WHITE = "\033[97m";   ///< Bright white text

    // ===============================
    // Task-Specific Utility Functions
    // ===============================

    /**
     * @brief Parse task status from string
     * @param statusStr Status string (case-insensitive)
     * @return TaskStatus enum value
     * @throws std::invalid_argument if status is invalid
     */
    [[nodiscard]] TaskStatus parseTaskStatus(std::string_view statusStr);

    /**
     * @brief Parse task priority from string
     * @param priorityStr Priority string (case-insensitive)
     * @return TaskPriority enum value
     * @throws std::invalid_argument if priority is invalid
     */
    [[nodiscard]] TaskPriority parseTaskPriority(std::string_view priorityStr);

    /**
     * @brief Get color for task status display
     * @param status Task status
     * @return ANSI color code string_view
     */
    [[nodiscard]] std::string_view getStatusColor(TaskStatus status);

    /**
     * @brief Get color for task priority display
     * @param priority Task priority
     * @return ANSI color code string_view
     */
    [[nodiscard]] std::string_view getPriorityColor(TaskPriority priority);

    // ===============================
    // Advanced Utility Functions
    // ===============================

    /**
     * @brief Convert bytes to human-readable size
     * @param bytes Number of bytes
     * @return Formatted string (e.g., "1.5 MB")
     */
    [[nodiscard]] std::string formatFileSize(size_t bytes);

    /**
     * @brief Generate random string for temporary files
     * @param length Length of random string
     * @return Random alphanumeric string
     */
    [[nodiscard]] std::string generateRandomString(size_t length = 8);

    /**
     * @brief Truncate string to maximum length with ellipsis
     * @param str String to truncate
     * @param max_length Maximum length including ellipsis
     * @return Truncated string with "..." if needed
     */
    [[nodiscard]] std::string truncateString(std::string_view str, size_t max_length);

} // namespace Utils

#endif // UTILS_HPP
