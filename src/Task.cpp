/**
 * @file Task.cpp
 * @brief Implementation of the Task class with modern C++ features
 *
 * This file implements all Task class functionality including:
 * - Fast enum parsing using compile-time hashing
 * - Comprehensive task property management
 * - JSON serialization/deserialization
 * - Date/time handling and validation
 * - Tag management system
 * - Display and formatting utilities
 */

#include "Task.hpp"
#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>

 // ======================================
 // Compile-time String Hashing for Fast Enum Lookups
 // ======================================

 /**
  * @brief Compile-time string hashing using FNV-1a algorithm
  * @param str String view to hash
  * @return 64-bit hash value
  *
  * This function enables compile-time hash computation for string literals,
  * allowing fast enum parsing through switch statements instead of string comparisons.
  */
constexpr uint64_t constexpr_hash(std::string_view str) noexcept {
    uint64_t hash = 14695981039346656037ULL; // FNV offset basis
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL; // FNV prime
    }
    return hash;
}

/**
 * @brief Fast status parsing using compile-time hash map - Phase 1 optimization
 * @param statusStr String representation of task status
 * @return TaskStatus enum value
 * @throws std::invalid_argument if status string is invalid
 *
 * Uses compile-time hashing to enable O(1) status parsing through switch statements.
 * Supports multiple string formats for each status (e.g., "todo", "1", etc.)
 */
TaskStatus parseTaskStatusFast(std::string_view statusStr) {
    switch (constexpr_hash(statusStr)) {
    case constexpr_hash("todo"):
    case constexpr_hash("1"):
        return TaskStatus::TODO;
    case constexpr_hash("inprogress"):
    case constexpr_hash("in-progress"):
    case constexpr_hash("2"):
        return TaskStatus::IN_PROGRESS;
    case constexpr_hash("completed"):
    case constexpr_hash("done"):
    case constexpr_hash("3"):
        return TaskStatus::COMPLETED;
    default:
        throw std::invalid_argument("Invalid status");
    }
}

/**
 * @brief Fast priority parsing using compile-time hash map - Phase 1 optimization
 * @param priorityStr String representation of task priority
 * @return TaskPriority enum value
 * @throws std::invalid_argument if priority string is invalid
 *
 * Uses compile-time hashing for efficient priority parsing.
 * Supports multiple string formats and abbreviations.
 */
TaskPriority parseTaskPriorityFast(std::string_view priorityStr) {
    switch (constexpr_hash(priorityStr)) {
    case constexpr_hash("low"):
    case constexpr_hash("1"):
        return TaskPriority::LOW;
    case constexpr_hash("medium"):
    case constexpr_hash("med"):
    case constexpr_hash("2"):
        return TaskPriority::MEDIUM;
    case constexpr_hash("high"):
    case constexpr_hash("3"):
        return TaskPriority::HIGH;
    default:
        throw std::invalid_argument("Invalid priority");
    }
}

// =========================
// Enum Conversion Utilities
// =========================

/**
 * @brief Convert integer to TaskStatus enum with validation
 * @param status Integer status value (1-3)
 * @return TaskStatus enum value
 * @throws std::invalid_argument if status value is out of range
 */
TaskStatus intToTaskStatus(int status) {
    switch (status) {
    case 1: return TaskStatus::TODO;
    case 2: return TaskStatus::IN_PROGRESS;
    case 3: return TaskStatus::COMPLETED;
    default: throw std::invalid_argument("Invalid status value: " + std::to_string(status));
    }
}

/**
 * @brief Convert integer to TaskPriority enum with validation
 * @param priority Integer priority value (1-3)
 * @return TaskPriority enum value
 * @throws std::invalid_argument if priority value is out of range
 */
TaskPriority intToTaskPriority(int priority) {
    switch (priority) {
    case 1: return TaskPriority::LOW;
    case 2: return TaskPriority::MEDIUM;
    case 3: return TaskPriority::HIGH;
    default: throw std::invalid_argument("Invalid priority value: " + std::to_string(priority));
    }
}

/**
 * @brief Convert TaskStatus enum to integer (noexcept)
 * @param status TaskStatus enum value
 * @return Integer representation (1-3)
 */
int taskStatusToInt(TaskStatus status) noexcept {
    return static_cast<int>(status);
}

/**
 * @brief Convert TaskPriority enum to integer (noexcept)
 * @param priority TaskPriority enum value
 * @return Integer representation (1-3)
 */
int taskPriorityToInt(TaskPriority priority) noexcept {
    return static_cast<int>(priority);
}

// ==========================================
// Task Class Implementation
// ==========================================

/**
 * @brief Construct Task with validation and automatic timestamp
 * @param id Unique task identifier
 * @param name Task name (validated non-empty)
 * @param status Initial task status
 * @param priority Initial task priority
 * @throws std::invalid_argument if name is empty
 */
Task::Task(int id, std::string_view name, TaskStatus status, TaskPriority priority)
    : id(id), name(name), status(status), priority(priority),
    created_at(std::chrono::system_clock::now()) {
    if (name.empty()) {
        throw std::invalid_argument("Task name cannot be empty");
    }
}

// =================
// Property Getters (all noexcept for performance)
// =================

int Task::getId() const noexcept {
    return id;
}

const std::string& Task::getName() const noexcept {
    return name;
}

TaskStatus Task::getStatus() const noexcept {
    return status;
}

TaskPriority Task::getPriority() const noexcept {
    return priority;
}

const std::chrono::system_clock::time_point& Task::getCreatedAt() const noexcept {
    return created_at;
}

const std::optional<std::chrono::system_clock::time_point>& Task::getCompletedAt() const noexcept {
    return completed_at;
}

const std::optional<std::chrono::system_clock::time_point>& Task::getDueDate() const noexcept {
    return due_date;
}

const std::string& Task::getDescription() const noexcept {
    return description;
}

const std::vector<std::string>& Task::getTags() const noexcept {
    return tags;
}

// =========================
// Property Setters with Validation and Side Effects
// =========================

/**
 * @brief Set task name with validation
 * @param name New task name
 * @throws std::invalid_argument if name is empty
 */
void Task::setName(std::string_view name) {
    if (name.empty()) {
        throw std::invalid_argument("Task name cannot be empty");
    }
    this->name = name;
}

/**
 * @brief Set task status with automatic completion timestamp management
 * @param status New task status
 *
 * Automatically sets completion timestamp when status changes to COMPLETED
 * and clears it when status changes away from COMPLETED.
 */
void Task::setStatus(TaskStatus status) {
    TaskStatus old_status = this->status;
    this->status = status;

    // Auto-manage completion timestamp based on status changes
    if (status == TaskStatus::COMPLETED && old_status != TaskStatus::COMPLETED) {
        completed_at = std::chrono::system_clock::now();
    }
    else if (status != TaskStatus::COMPLETED) {
        completed_at.reset(); // Clear completion time if moving away from completed
    }
}

void Task::setPriority(TaskPriority priority) {
    this->priority = priority;
}

void Task::setDescription(std::string_view description) {
    this->description = description;
}

void Task::setDueDate(const std::optional<std::chrono::system_clock::time_point>& due_date) {
    this->due_date = due_date;
}

// ================
// Tag Management System
// ================

/**
 * @brief Add tag to task (prevents duplicates)
 * @param tag Tag string to add
 *
 * Only adds the tag if it's non-empty and not already present.
 * Prevents duplicate tags in the collection.
 */
void Task::addTag(std::string_view tag) {
    std::string tag_str{ tag };
    if (!tag_str.empty() && !hasTag(tag)) {
        tags.push_back(std::move(tag_str));
    }
}

/**
 * @brief Remove tag from task
 * @param tag Tag string to remove
 *
 * Uses std::ranges::find for efficient tag lookup and removal.
 */
void Task::removeTag(std::string_view tag) {
    auto it = std::ranges::find(tags, tag);
    if (it != tags.end()) {
        tags.erase(it);
    }
}

/**
 * @brief Check if task has specific tag
 * @param tag Tag string to search for
 * @return true if tag exists, false otherwise
 */
bool Task::hasTag(std::string_view tag) const {
    return std::ranges::find(tags, tag) != tags.end();
}

// ===================
// Status and Utility Methods
// ===================

/**
 * @brief Get human-readable status string
 * @return Formatted status string
 */
std::string Task::getStatusString() const {
    switch (status) {
    case TaskStatus::TODO: return "To-Do";
    case TaskStatus::IN_PROGRESS: return "In Progress";
    case TaskStatus::COMPLETED: return "Completed";
    default: return "Unknown";
    }
}

/**
 * @brief Get human-readable priority string
 * @return Formatted priority string
 */
std::string Task::getPriorityString() const {
    switch (priority) {
    case TaskPriority::LOW: return "Low";
    case TaskPriority::MEDIUM: return "Medium";
    case TaskPriority::HIGH: return "High";
    default: return "Unknown";
    }
}

/**
 * @brief Get formatted creation date/time
 * @return Human-readable creation timestamp
 */
std::string Task::getFormattedCreatedAt() const {
    return Utils::formatDateTime(created_at);
}

/**
 * @brief Get formatted due date or default message
 * @return Formatted due date or "No due date"
 */
std::string Task::getFormattedDueDate() const {
    if (due_date) {
        return Utils::formatDate(*due_date);
    }
    return "No due date";
}

/**
 * @brief Check if task is overdue
 * @return true if task has due date in the past and is not completed
 *
 * A task is considered overdue if:
 * 1. It has a due date set
 * 2. Current time is past the due date
 * 3. Task is not completed
 */
bool Task::isOverdue() const {
    if (!due_date) return false;
    return std::chrono::system_clock::now() > *due_date && status != TaskStatus::COMPLETED;
}

/**
 * @brief Calculate days until due date
 * @return Number of days (can be negative if overdue)
 *
 * Returns 0 if no due date is set.
 * Negative values indicate overdue tasks.
 */
std::chrono::days Task::getDaysUntilDue() const {
    if (!due_date) return std::chrono::days{ 0 };
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::days>(*due_date - now);
}

/**
 * @brief Mark task as completed (convenience method)
 *
 * Sets status to COMPLETED which automatically updates completion timestamp.
 */
void Task::markCompleted() {
    setStatus(TaskStatus::COMPLETED);
}

/**
 * @brief Check if task matches search query
 * @param query Search query string
 * @return true if task matches query in name, description, or tags
 *
 * Performs case-insensitive search across:
 * - Task name
 * - Task description
 * - All tags
 */
bool Task::matches(std::string_view query) const {
    std::string query_lower = Utils::toLowerCase(query);
    std::string name_lower = Utils::toLowerCase(name);
    std::string desc_lower = Utils::toLowerCase(description);

    // Check name and description for matches
    if (Utils::contains(name_lower, query_lower) || Utils::contains(desc_lower, query_lower)) {
        return true;
    }

    // Check all tags for matches
    return std::ranges::any_of(tags, [&](const std::string& tag) {
        return Utils::contains(Utils::toLowerCase(tag), query_lower);
        });
}

// ============================
// JSON Serialization Support
// ============================

/**
 * @brief Convert task to JSON for persistence
 * @return JSON object representing the task
 *
 * Serializes all task properties including optional fields.
 * Uses Unix timestamp format for time points.
 */
nlohmann::json Task::toJson() const {
    nlohmann::json j{
        {"id", id},
        {"name", name},
        {"status", taskStatusToInt(status)},
        {"priority", taskPriorityToInt(priority)},
        {"created_at", std::chrono::duration_cast<std::chrono::seconds>(created_at.time_since_epoch()).count()},
        {"description", description},
        {"tags", tags}
    };

    // Add optional completion timestamp
    if (completed_at) {
        j["completed_at"] = std::chrono::duration_cast<std::chrono::seconds>(completed_at->time_since_epoch()).count();
    }

    // Add optional due date
    if (due_date) {
        j["due_date"] = std::chrono::duration_cast<std::chrono::seconds>(due_date->time_since_epoch()).count();
    }

    return j;
}

/**
 * @brief Create task from JSON data
 * @param j JSON object containing task data
 * @return Task object reconstructed from JSON
 * @throws Various exceptions if JSON is malformed
 *
 * Deserializes task from JSON format with proper error handling.
 * Handles optional fields gracefully.
 */
Task Task::fromJson(const nlohmann::json& j) {
    // Create basic task from required fields
    Task task(
        j.at("id").get<int>(),
        j.at("name").get<std::string>(),
        intToTaskStatus(j.at("status").get<int>()),
        intToTaskPriority(j.at("priority").get<int>())
    );

    // Restore creation timestamp
    if (j.contains("created_at")) {
        auto seconds = std::chrono::seconds{ j.at("created_at").get<int64_t>() };
        task.created_at = std::chrono::system_clock::time_point{ seconds };
    }

    // Restore optional completion timestamp
    if (j.contains("completed_at")) {
        auto seconds = std::chrono::seconds{ j.at("completed_at").get<int64_t>() };
        task.completed_at = std::chrono::system_clock::time_point{ seconds };
    }

    // Restore optional due date
    if (j.contains("due_date")) {
        auto seconds = std::chrono::seconds{ j.at("due_date").get<int64_t>() };
        task.due_date = std::chrono::system_clock::time_point{ seconds };
    }

    // Restore optional description and tags
    if (j.contains("description")) {
        task.description = j.at("description").get<std::string>();
    }

    if (j.contains("tags")) {
        task.tags = j.at("tags").get<std::vector<std::string>>();
    }

    return task;
}

// =================
// Display Methods
// =================

/**
 * @brief Get compact string representation for table display
 * @return Formatted string with essential task info
 *
 * Provides a single-line representation suitable for table rows.
 * Includes color coding and overdue indicators.
 */
std::string Task::toString() const {
    std::ostringstream oss;

    // Get appropriate colors for visual feedback
    auto statusColor = Utils::getStatusColor(status);
    auto priorityColor = Utils::getPriorityColor(priority);

    // Add overdue warning indicator
    std::string overdueIndicator = isOverdue() ? " ⚠️" : "";

    // Format with consistent column widths
    oss << std::left << std::setw(4) << id
        << std::setw(30) << (name + overdueIndicator)
        << statusColor << std::setw(12) << getStatusString() << Utils::RESET
        << priorityColor << std::setw(8) << getPriorityString() << Utils::RESET;

    // Add due date if present
    if (due_date) {
        oss << " (Due: " << getFormattedDueDate() << ")";
    }

    return oss.str();
}

/**
 * @brief Get detailed string representation for full task view
 * @return Multi-line formatted string with all task details
 *
 * Provides comprehensive task information including all metadata.
 * Used for detailed task display commands.
 */
std::string Task::toDetailedString() const {
    std::ostringstream oss;

    oss << Utils::BOLD << "Task #" << id << ": " << name << Utils::RESET << "\n";
    oss << "Status: " << Utils::getStatusColor(status) << getStatusString() << Utils::RESET << "\n";
    oss << "Priority: " << Utils::getPriorityColor(priority) << getPriorityString() << Utils::RESET << "\n";
    oss << "Created: " << getFormattedCreatedAt() << "\n";

    if (completed_at) {
        oss << "Completed: " << Utils::formatDateTime(*completed_at) << "\n";
    }

    if (due_date) {
        oss << "Due Date: " << getFormattedDueDate();
        if (isOverdue()) {
            oss << " " << Utils::RED << "(OVERDUE)" << Utils::RESET;
        }
        oss << "\n";
    }

    if (!description.empty()) {
        oss << "Description: " << description << "\n";
    }

    if (!tags.empty()) {
        oss << "Tags: ";
        for (size_t i = 0; i < tags.size(); ++i) {
            oss << Utils::BLUE << "#" << tags[i] << Utils::RESET;
            if (i < tags.size() - 1) oss << ", ";
        }
        oss << "\n";
    }

    return oss.str();
}

// ==================
// Comparison Operators
// ==================

/**
 * @brief Equality comparison based on task ID
 * @param other Task to compare with
 * @return true if tasks have same ID
 */
bool Task::operator==(const Task& other) const noexcept {
    return id == other.id;
}

/**
 * @brief Less-than comparison for sorting
 * @param other Task to compare with
 * @return true if this task should come before other in sorted order
 *
 * Sorting priority:
 * 1. Priority (high first)
 * 2. Due date (earlier first, no due date last)
 * 3. Creation date (earlier first)
 */
bool Task::operator<(const Task& other) const noexcept {
    // Sort by priority (high first)
    if (priority != other.priority) {
        return priority > other.priority; // Higher priority comes first
    }

    // Then by due date (earlier due dates first, no due date last)
    if (due_date != other.due_date) {
        if (!due_date) return false;     // No due date goes last
        if (!other.due_date) return true; // Has due date goes first
        return due_date < other.due_date;  // Earlier due date first
    }

    // Finally by creation date (earlier first)
    return created_at < other.created_at;
}
