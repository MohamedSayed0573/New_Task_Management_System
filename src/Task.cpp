#include "Task.hpp"
#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <vector>
#include <algorithm>
#include <stdexcept>
#include <format>

// Compile-time string hashing for fast enum lookups - Phase 1 optimization
constexpr uint64_t constexpr_hash(std::string_view str) noexcept {
    uint64_t hash = 14695981039346656037ULL;
    for (char c : str) {
        hash ^= static_cast<uint64_t>(c);
        hash *= 1099511628211ULL;
    }
    return hash;
}

// Fast status parsing using compile-time hash map - Phase 1 optimization
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

// Fast priority parsing using compile-time hash map - Phase 1 optimization
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

// Utility functions for enum conversions
TaskStatus intToTaskStatus(int status) {
    switch (status) {
        case 1: return TaskStatus::TODO;
        case 2: return TaskStatus::IN_PROGRESS;
        case 3: return TaskStatus::COMPLETED;
        default: throw std::invalid_argument("Invalid status value: " + std::to_string(status));
    }
}

TaskPriority intToTaskPriority(int priority) {
    switch (priority) {
        case 1: return TaskPriority::LOW;
        case 2: return TaskPriority::MEDIUM;
        case 3: return TaskPriority::HIGH;
        default: throw std::invalid_argument("Invalid priority value: " + std::to_string(priority));
    }
}

int taskStatusToInt(TaskStatus status) noexcept {
    return static_cast<int>(status);
}

int taskPriorityToInt(TaskPriority priority) noexcept {
    return static_cast<int>(priority);
}

// Task implementation
Task::Task(int id, std::string_view name, TaskStatus status, TaskPriority priority)
    : id(id), name(name), status(status), priority(priority), 
      created_at(std::chrono::system_clock::now()) {
    if (name.empty()) {
        throw std::invalid_argument("Task name cannot be empty");
    }
}

// Getters
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

// Setters with validation
void Task::setName(std::string_view name) {
    if (name.empty()) {
        throw std::invalid_argument("Task name cannot be empty");
    }
    this->name = name;
}

void Task::setStatus(TaskStatus status) {
    TaskStatus old_status = this->status;
    this->status = status;
    
    if (status == TaskStatus::COMPLETED && old_status != TaskStatus::COMPLETED) {
        completed_at = std::chrono::system_clock::now();
    } else if (status != TaskStatus::COMPLETED) {
        completed_at.reset();
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

// Tag management
void Task::addTag(std::string_view tag) {
    std::string tag_str{tag};
    if (!tag_str.empty() && !hasTag(tag)) {
        tags.push_back(std::move(tag_str));
    }
}

void Task::removeTag(std::string_view tag) {
    auto it = std::ranges::find(tags, tag);
    if (it != tags.end()) {
        tags.erase(it);
    }
}

bool Task::hasTag(std::string_view tag) const {
    return std::ranges::find(tags, tag) != tags.end();
}

// Status and utility methods
std::string Task::getStatusString() const {
    switch (status) {
        case TaskStatus::TODO: return "To-Do";
        case TaskStatus::IN_PROGRESS: return "In Progress";
        case TaskStatus::COMPLETED: return "Completed";
        default: return "Unknown";
    }
}

std::string Task::getPriorityString() const {
    switch (priority) {
        case TaskPriority::LOW: return "Low";
        case TaskPriority::MEDIUM: return "Medium";
        case TaskPriority::HIGH: return "High";
        default: return "Unknown";
    }
}

std::string Task::getFormattedCreatedAt() const {
    return Utils::formatDateTime(created_at);
}

std::string Task::getFormattedDueDate() const {
    if (due_date) {
        return Utils::formatDate(*due_date);
    }
    return "No due date";
}

bool Task::isOverdue() const {
    if (!due_date) return false;
    return std::chrono::system_clock::now() > *due_date && status != TaskStatus::COMPLETED;
}

std::chrono::days Task::getDaysUntilDue() const {
    if (!due_date) return std::chrono::days{0};
    auto now = std::chrono::system_clock::now();
    return std::chrono::duration_cast<std::chrono::days>(*due_date - now);
}

void Task::markCompleted() {
    setStatus(TaskStatus::COMPLETED);
}

bool Task::matches(std::string_view query) const {
    std::string query_lower = Utils::toLowerCase(query);
    std::string name_lower = Utils::toLowerCase(name);
    std::string desc_lower = Utils::toLowerCase(description);
    
    // Check name and description
    if (Utils::contains(name_lower, query_lower) || Utils::contains(desc_lower, query_lower)) {
        return true;
    }
    
    // Check tags
    return std::ranges::any_of(tags, [&](const std::string& tag) {
        return Utils::contains(Utils::toLowerCase(tag), query_lower);
    });
}

// JSON serialization
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
    
    if (completed_at) {
        j["completed_at"] = std::chrono::duration_cast<std::chrono::seconds>(completed_at->time_since_epoch()).count();
    }
    
    if (due_date) {
        j["due_date"] = std::chrono::duration_cast<std::chrono::seconds>(due_date->time_since_epoch()).count();
    }
    
    return j;
}

Task Task::fromJson(const nlohmann::json& j) {
    Task task(
        j.at("id").get<int>(),
        j.at("name").get<std::string>(),
        intToTaskStatus(j.at("status").get<int>()),
        intToTaskPriority(j.at("priority").get<int>())
    );
    
    // Set creation time
    if (j.contains("created_at")) {
        auto seconds = std::chrono::seconds{j.at("created_at").get<int64_t>()};
        task.created_at = std::chrono::system_clock::time_point{seconds};
    }
    
    // Set completion time
    if (j.contains("completed_at")) {
        auto seconds = std::chrono::seconds{j.at("completed_at").get<int64_t>()};
        task.completed_at = std::chrono::system_clock::time_point{seconds};
    }
    
    // Set due date
    if (j.contains("due_date")) {
        auto seconds = std::chrono::seconds{j.at("due_date").get<int64_t>()};
        task.due_date = std::chrono::system_clock::time_point{seconds};
    }
    
    // Set description and tags
    if (j.contains("description")) {
        task.description = j.at("description").get<std::string>();
    }
    
    if (j.contains("tags")) {
        task.tags = j.at("tags").get<std::vector<std::string>>();
    }
    
    return task;
}

// Display methods
std::string Task::toString() const {
    std::ostringstream oss;
    
    // Color coding based on status and priority
    auto statusColor = Utils::getStatusColor(status);
    auto priorityColor = Utils::getPriorityColor(priority);
    
    // Add overdue indicator
    std::string overdueIndicator = isOverdue() ? " ⚠️" : "";
    
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
            oss << Utils::RED << " (OVERDUE)" << Utils::RESET;
        }
        oss << "\n";
    }
    
    if (!description.empty()) {
        oss << "Description: " << description << "\n";
    }
    
    if (!tags.empty()) {
        oss << "Tags: ";
        for (size_t i = 0; i < tags.size(); ++i) {
            oss << Utils::CYAN << "#" << tags[i] << Utils::RESET;
            if (i < tags.size() - 1) oss << ", ";
        }
        oss << "\n";
    }
    
    return oss.str();
}

// Operators
bool Task::operator==(const Task& other) const noexcept {
    return id == other.id;
}

bool Task::operator<(const Task& other) const noexcept {
    // Sort by priority (high first), then by due date, then by creation date
    if (priority != other.priority) {
        return priority > other.priority;  // High priority first
    }
    
    if (due_date != other.due_date) {
        if (!due_date) return false;  // Tasks without due date go last
        if (!other.due_date) return true;
        return *due_date < *other.due_date;  // Earlier due dates first
    }
    
    return created_at < other.created_at;  // Earlier creation dates first
}
