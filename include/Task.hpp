#ifndef TASK_HPP
#define TASK_HPP

#include <string>
#include <chrono>
#include <optional>
#include <string_view>
#include <vector>
#include "json.hpp"

enum class TaskStatus : int {
    TODO = 1,
    IN_PROGRESS = 2,
    COMPLETED = 3
};

enum class TaskPriority : int {
    LOW = 1,
    MEDIUM = 2,
    HIGH = 3
};

class Task {
private:
    int id;
    std::string name;
    TaskStatus status;
    TaskPriority priority;
    std::chrono::system_clock::time_point created_at;
    std::optional<std::chrono::system_clock::time_point> completed_at;
    std::optional<std::chrono::system_clock::time_point> due_date;
    std::string description;
    std::vector<std::string> tags;

public:
    // Constructor with modern initialization
    Task(int id, std::string_view name, TaskStatus status = TaskStatus::TODO, 
         TaskPriority priority = TaskPriority::LOW);
    
    // Rule of 5 (modern C++)
    Task(const Task&) = default;
    Task(Task&&) = default;
    Task& operator=(const Task&) = default;
    Task& operator=(Task&&) = default;
    ~Task() = default;
    
    // Getters
    [[nodiscard]] int getId() const noexcept;
    [[nodiscard]] const std::string& getName() const noexcept;
    [[nodiscard]] TaskStatus getStatus() const noexcept;
    [[nodiscard]] TaskPriority getPriority() const noexcept;
    [[nodiscard]] const std::chrono::system_clock::time_point& getCreatedAt() const noexcept;
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& getCompletedAt() const noexcept;
    [[nodiscard]] const std::optional<std::chrono::system_clock::time_point>& getDueDate() const noexcept;
    [[nodiscard]] const std::string& getDescription() const noexcept;
    [[nodiscard]] const std::vector<std::string>& getTags() const noexcept;
    
    // Setters with validation
    void setName(std::string_view name);
    void setStatus(TaskStatus status);
    void setPriority(TaskPriority priority);
    void setDescription(std::string_view description);
    void setDueDate(const std::optional<std::chrono::system_clock::time_point>& due_date);
    
    // Tag management
    void addTag(std::string_view tag);
    void removeTag(std::string_view tag);
    [[nodiscard]] bool hasTag(std::string_view tag) const;
    
    // Status helper methods
    [[nodiscard]] std::string getStatusString() const;
    [[nodiscard]] std::string getPriorityString() const;
    [[nodiscard]] std::string getFormattedCreatedAt() const;
    [[nodiscard]] std::string getFormattedDueDate() const;
    [[nodiscard]] bool isOverdue() const;
    [[nodiscard]] std::chrono::days getDaysUntilDue() const;
    
    // Utility methods
    void markCompleted();
    [[nodiscard]] bool matches(std::string_view query) const;
    
    // JSON serialization
    [[nodiscard]] nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& j);
    
    // Display
    [[nodiscard]] std::string toString() const;
    [[nodiscard]] std::string toDetailedString() const;
    
    // Operators
    bool operator==(const Task& other) const noexcept;
    bool operator<(const Task& other) const noexcept;  // For sorting by priority then due date
};

// Utility functions for enum conversions
[[nodiscard]] TaskStatus intToTaskStatus(int status);
[[nodiscard]] TaskPriority intToTaskPriority(int priority);
[[nodiscard]] int taskStatusToInt(TaskStatus status) noexcept;
[[nodiscard]] int taskPriorityToInt(TaskPriority priority) noexcept;

#endif // TASK_HPP
