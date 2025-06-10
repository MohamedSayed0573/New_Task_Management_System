/**
 * @file Task.hpp
 * @brief Task class definition with modern C++ features
 *
 * This header defines the core Task class and related enums for the todo application.
 * Includes comprehensive task management functionality with validation, JSON serialization,
 * and modern C++20/23 features.
 */

#ifndef TASK_HPP
#define TASK_HPP

#include <string>
#include <chrono>
#include <optional>
#include <string_view>
#include <vector>
#include "json.hpp"

 /**
  * @enum TaskStatus
  * @brief Represents the current state of a task
  */
enum class TaskStatus : int {
    TODO = 1,        ///< Task is pending and not started
    IN_PROGRESS = 2, ///< Task is currently being worked on
    COMPLETED = 3    ///< Task has been finished
};

/**
 * @enum TaskPriority
 * @brief Represents the importance level of a task
 */
enum class TaskPriority : int {
    LOW = 1,    ///< Low importance task
    MEDIUM = 2, ///< Medium importance task  
    HIGH = 3    ///< High importance task - urgent
};

/**
 * @class Task
 * @brief Core task entity with comprehensive functionality
 *
 * Represents a single task with all associated metadata including:
 * - Basic info (ID, name, description)
 * - Status and priority management
 * - Date tracking (created, completed, due dates)
 * - Tag system for categorization
 * - JSON serialization for persistence
 */
class Task {
private:
    // Core task properties
    int id;                          ///< Unique identifier for the task
    std::string name;                ///< Task name/title
    TaskStatus status;               ///< Current status of the task
    TaskPriority priority;           ///< Priority level of the task

    // Timestamp management
    std::chrono::system_clock::time_point created_at;                           ///< When the task was created
    std::optional<std::chrono::system_clock::time_point> completed_at;          ///< When the task was completed (if applicable)
    std::optional<std::chrono::system_clock::time_point> due_date;              ///< When the task is due (optional)

    // Additional metadata
    std::string description;         ///< Detailed description of the task
    std::vector<std::string> tags;   ///< List of tags for categorization

public:
    // ===========================
    // Constructors and Destructors
    // ===========================

    /**
     * @brief Construct a new Task with validation
     * @param id Unique task identifier
     * @param name Task name (must not be empty)
     * @param status Initial status (default: TODO)
     * @param priority Initial priority (default: LOW)
     * @throws std::invalid_argument if name is empty
     */
    Task(int id, std::string_view name, TaskStatus status = TaskStatus::TODO,
        TaskPriority priority = TaskPriority::LOW);

    // Rule of 5 for modern C++ - explicit defaults for compiler-generated functions
    Task(const Task&) = default;               ///< Copy constructor
    Task(Task&&) = default;                    ///< Move constructor  
    Task& operator=(const Task&) = default;    ///< Copy assignment
    Task& operator=(Task&&) = default;         ///< Move assignment
    ~Task() = default;                         ///< Destructor

    // =================
    // Property Getters
    // =================

    [[nodiscard]] int getId() const noexcept;                                                                  ///< Get task ID
    [[nodiscard]] const std::string& getName() const noexcept;                                                ///< Get task name
    [[nodiscard]] TaskStatus getStatus() const noexcept;                                                      ///< Get task status
    [[nodiscard]] TaskPriority getPriority() const noexcept;                                                  ///< Get task priority
    [[nodiscard]] const std::chrono::system_clock::time_point& getCreatedAt() const noexcept;                ///< Get creation timestamp
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& getCompletedAt() const noexcept; ///< Get completion timestamp
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& getDueDate() const noexcept;    ///< Get due date
    [[nodiscard]] const std::string& getDescription() const noexcept;                                         ///< Get task description
    [[nodiscard]] const std::vector<std::string>& getTags() const noexcept;                                   ///< Get list of tags

    // =========================
    // Property Setters with Validation
    // =========================

    void setName(std::string_view name);                                                           ///< Set task name (validates non-empty)
    void setStatus(TaskStatus status);                                                             ///< Set status (auto-updates completion time)
    void setPriority(TaskPriority priority);                                                       ///< Set priority level
    void setDescription(std::string_view description);                                             ///< Set task description
    void setDueDate(const std::optional<std::chrono::system_clock::time_point>& due_date);        ///< Set due date (optional)

    // ================
    // Tag Management
    // ================

    void addTag(std::string_view tag);           ///< Add a tag to the task (prevents duplicates)
    void removeTag(std::string_view tag);        ///< Remove a tag from the task
    [[nodiscard]] bool hasTag(std::string_view tag) const; ///< Check if task has a specific tag

    // ===================
    // Status and Utilities
    // ===================

    [[nodiscard]] std::string getStatusString() const;      ///< Get human-readable status
    [[nodiscard]] std::string getPriorityString() const;    ///< Get human-readable priority
    [[nodiscard]] std::string getFormattedCreatedAt() const; ///< Get formatted creation date
    [[nodiscard]] std::string getFormattedDueDate() const;  ///< Get formatted due date
    [[nodiscard]] bool isOverdue() const;                   ///< Check if task is past due date
    [[nodiscard]] std::chrono::days getDaysUntilDue() const; ///< Get days until due (can be negative)

    // ==================
    // Convenience Methods
    // ==================

    void markCompleted();                                    ///< Mark task as completed (sets status and completion time)
    [[nodiscard]] bool matches(std::string_view query) const; ///< Check if task matches search query

    // ====================
    // Serialization Support
    // ====================

    [[nodiscard]] nlohmann::json toJson() const;            ///< Convert task to JSON for persistence
    static Task fromJson(const nlohmann::json& j);          ///< Create task from JSON data

    // =================
    // Display Methods
    // =================

    [[nodiscard]] std::string toString() const;             ///< Get compact string representation
    [[nodiscard]] std::string toDetailedString() const;     ///< Get detailed string representation

    // ==================
    // Comparison Operators
    // ==================

    bool operator==(const Task& other) const noexcept;      ///< Equality comparison (by ID)
    bool operator<(const Task& other) const noexcept;       ///< Less-than comparison (for sorting by priority/due date)
};

// ===============================
// Utility Functions for Enum Conversion
// ===============================

[[nodiscard]] TaskStatus intToTaskStatus(int status);       ///< Convert integer to TaskStatus enum
[[nodiscard]] TaskPriority intToTaskPriority(int priority); ///< Convert integer to TaskPriority enum
[[nodiscard]] int taskStatusToInt(TaskStatus status) noexcept;    ///< Convert TaskStatus to integer
[[nodiscard]] int taskPriorityToInt(TaskPriority priority) noexcept; ///< Convert TaskPriority to integer

// Fast parsing functions using constexpr hashing - Phase 1 optimization
[[nodiscard]] TaskStatus parseTaskStatusFast(std::string_view statusStr);     ///< Fast status parsing using compile-time hashing
[[nodiscard]] TaskPriority parseTaskPriorityFast(std::string_view priorityStr); ///< Fast priority parsing using compile-time hashing

#endif // TASK_HPP
