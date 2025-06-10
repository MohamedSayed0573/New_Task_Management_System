/**
 * @file Tasks.hpp
 * @brief Tasks container class with advanced features and optimizations
 *
 * This header defines the Tasks class which manages a collection of Task objects.
 * Includes modern C++ features like search indexing, statistics caching,
 * and comprehensive task management functionality.
 */

#ifndef TASKS_HPP
#define TASKS_HPP

#include "Task.hpp"
#include "TaskSearchIndex.hpp"
#include <vector>
#include <string>
#include <string_view>
#include <memory>
#include <functional>
#include <span>
#include <ranges>
#include <algorithm>
#include <filesystem>
#include <optional>

 /**
  * @struct TaskResult
  * @brief Wrapper for task operation results with success/error information
  *
  * Provides a standardized way to return operation results with both
  * success status and descriptive messages for user feedback.
  */
struct TaskResult {
    bool success;           ///< Whether the operation succeeded
    std::string message;    ///< Descriptive message about the operation result

    /**
     * @brief Create a successful result
     * @param msg Success message
     * @return TaskResult with success=true
     */
    static TaskResult successResult(std::string_view msg) {
        return { true, std::string{msg} };
    }

    /**
     * @brief Create an error result
     * @param msg Error message
     * @return TaskResult with success=false
     */
    static TaskResult errorResult(std::string_view msg) {
        return { false, std::string{msg} };
    }
};

/**
 * @struct TaskStats
 * @brief Statistical information about tasks in the collection
 *
 * Provides comprehensive statistics for dashboard and reporting functionality.
 * All counters are updated lazily for performance optimization.
 */
struct TaskStats {
    size_t total = 0;         ///< Total number of tasks
    size_t todo = 0;          ///< Number of TODO tasks
    size_t inProgress = 0;    ///< Number of IN_PROGRESS tasks
    size_t completed = 0;     ///< Number of COMPLETED tasks
    size_t lowPriority = 0;   ///< Number of LOW priority tasks
    size_t mediumPriority = 0; ///< Number of MEDIUM priority tasks
    size_t highPriority = 0;  ///< Number of HIGH priority tasks
    size_t overdue = 0;       ///< Number of overdue tasks
};

/**
 * @class Tasks
 * @brief Main container class for managing a collection of tasks
 *
 * This class provides comprehensive task management functionality including:
 * - CRUD operations for tasks
 * - Advanced search and filtering capabilities
 * - Statistics computation with caching
 * - Multiple display formats and export options
 *
 * Phase 2 optimizations include:
 * - Search index for fast queries
 * - Lazy statistics computation
 * - Memory-efficient operations
 */
class Tasks {
private:
    // ==================
    // Core Data Storage
    // ==================

    std::vector<std::unique_ptr<Task>> tasks;    ///< Main task storage using smart pointers
    int nextId;                                   ///< Next available task ID
    std::filesystem::path dataFile;               ///< Path to JSON data file

    // =============================
    // Phase 2 Optimization Features
    // =============================

    mutable TaskSearchIndex search_index_;       ///< Advanced search index for fast queries
    mutable bool index_dirty_ = true;            ///< Flag to rebuild search index when needed

    mutable std::optional<TaskStats> cached_stats_; ///< Cached statistics to avoid recomputation
    mutable bool stats_dirty_ = true;               ///< Flag to recalculate stats when needed

    // ===================
    // Internal Helper Methods
    // ===================

    void loadFromFile();                         ///< Load tasks from JSON file
    void saveToFile() const;                     ///< Synchronous save to JSON file
    void rebuildSearchIndex() const;             ///< Rebuild search index when dirty
    [[nodiscard]] std::vector<Task*> getSortedTasks() const; ///< Get tasks sorted by priority and due date

    // ========================
    // Table Display Helpers
    // ========================

    void printTableSeparator(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const;
    void printTableHeader(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const;
    void printTaskRow(const Task* task, int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const;
    std::string formatTaskName(const Task* task, int maxWidth) const;

public:
    // ===========================
    // Constructors and Destructors  
    // ===========================

    /**
     * @brief Construct Tasks container with data file path
     * @param dataFile Path to JSON file for persistence (default: "data/data.json")
     */
    explicit Tasks(std::filesystem::path dataFile = "data/data.json");

    // Rule of 5 for proper resource management
    Tasks(const Tasks&) = delete;               ///< Disable copying (unique ownership)
    Tasks(Tasks&&) = default;                   ///< Enable moving
    Tasks& operator=(const Tasks&) = delete;    ///< Disable copy assignment
    Tasks& operator=(Tasks&&) = default;        ///< Enable move assignment
    ~Tasks() = default;                         ///< Default destructor

    // =================
    // Display Utilities
    // =================

    /**
     * @brief Display a list of tasks in formatted table
     * @param taskList List of task pointers to display
     * @param title Optional title for the table
     */
    void displayTaskList(const std::vector<Task*>& taskList, std::string_view title = "") const;

    // ====================================
    // Task Management with Error Handling
    // ====================================

    [[nodiscard]] TaskResult addTask(std::string_view name, TaskStatus status = TaskStatus::TODO,
        TaskPriority priority = TaskPriority::LOW);                                     ///< Add simple task
    [[nodiscard]] TaskResult addTask(std::string_view name, std::string_view description,
        TaskStatus status = TaskStatus::TODO, TaskPriority priority = TaskPriority::LOW,
        const std::optional<std::chrono::system_clock::time_point>& dueDate = std::nullopt,
        const std::vector<std::string>& tags = {});                                     ///< Add task with full details
    [[nodiscard]] TaskResult removeTask(int id);                                       ///< Remove task by ID
    [[nodiscard]] TaskResult removeAllTasks();                                         ///< Remove all tasks
    [[nodiscard]] TaskResult updateTask(int id, std::string_view name, TaskStatus status, TaskPriority priority); ///< Update existing task

    // ================
    // Task Retrieval
    // ================

    [[nodiscard]] Task* findTask(int id) noexcept;                                     ///< Find task by ID (mutable)
    [[nodiscard]] const Task* findTask(int id) const noexcept;                        ///< Find task by ID (const)
    [[nodiscard]] std::vector<Task*> searchTasks(std::string_view query) const;       ///< Basic search functionality
    [[nodiscard]] std::vector<Task*> advancedSearch(std::string_view query) const;    ///< Advanced search using index
    [[nodiscard]] std::vector<Task*> getTasksByStatus(TaskStatus status) const;       ///< Filter by status
    [[nodiscard]] std::vector<Task*> getTasksByPriority(TaskPriority priority) const; ///< Filter by priority
    [[nodiscard]] std::vector<Task*> getTasksByTag(std::string_view tag) const;       ///< Filter by tag
    [[nodiscard]] std::vector<Task*> getOverdueTasks() const;                         ///< Get overdue tasks

    // ===========
    // Statistics
    // ===========

    /**
     * @brief Get comprehensive task statistics
     * @return TaskStats object with current statistics
     * @note Uses lazy computation with caching for performance
     */
    [[nodiscard]] TaskStats getStatistics() const;

    // =================
    // Data Persistence
    // =================

    void save() const;                                                                  ///< Save tasks to file

    // ====================================
    // Display Methods with Enhanced Formatting
    // ====================================

    void showAllTasks() const;                                                          ///< Display all tasks in table format
    void showFilteredTasks(TaskStatus status) const;                                   ///< Display tasks filtered by status
    void showFilteredTasks(TaskPriority priority) const;                               ///< Display tasks filtered by priority
    void showTaskDetails(int id) const;                                                ///< Show detailed view of specific task
    void showOverdueTasks() const;                                                     ///< Display overdue tasks with warnings
    void showStatistics() const;                                                       ///< Display comprehensive statistics dashboard

    // =================
    // Utility Methods
    // =================

    [[nodiscard]] int getNextId() const noexcept;                                      ///< Get next available task ID
    [[nodiscard]] bool isEmpty() const noexcept;                                       ///< Check if container is empty
    [[nodiscard]] size_t size() const noexcept;                                        ///< Get number of tasks
};

#endif // TASKS_HPP
