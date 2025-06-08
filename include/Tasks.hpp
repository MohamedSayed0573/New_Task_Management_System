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
#include <future>

// Task operation result wrapper
struct TaskResult {
    bool success;
    std::string message;
    
    static TaskResult successResult(std::string_view msg) {
        return {true, std::string{msg}};
    }
    
    static TaskResult errorResult(std::string_view msg) {
        return {false, std::string{msg}};
    }
};

// Task statistics structure
struct TaskStats {
    size_t total = 0;
    size_t todo = 0;
    size_t inProgress = 0;
    size_t completed = 0;
    size_t lowPriority = 0;
    size_t mediumPriority = 0;
    size_t highPriority = 0;
    size_t overdue = 0;
};

class Tasks {
private:
    std::vector<std::unique_ptr<Task>> tasks;
    int nextId;
    std::filesystem::path dataFile;
    
    // Phase 2 optimization: Advanced search index
    mutable TaskSearchIndex search_index_;
    mutable bool index_dirty_ = true;
    
    // Phase 2 optimization: Lazy statistics computation
    mutable std::optional<TaskStats> cached_stats_;
    mutable bool stats_dirty_ = true;
    
    // Phase 2 optimization: Async file I/O
    mutable std::future<void> save_future_;
    
    // Internal helper methods
    void loadFromFile();
    void saveToFile() const;
    void saveAsync() const;
    void waitForSave() const;
    void rebuildSearchIndex() const;
    [[nodiscard]] std::vector<Task*> getSortedTasks() const;
    
public:
    // Constructor with path validation
    explicit Tasks(std::filesystem::path dataFile = "data/data.json");
    
    // Rule of 5 for proper resource management
    Tasks(const Tasks&) = delete;  // Disable copying
    Tasks(Tasks&&) = default;
    Tasks& operator=(const Tasks&) = delete;
    Tasks& operator=(Tasks&&) = default;
    ~Tasks() = default;
    
    // Display utilities
    void displayTaskList(const std::vector<Task*>& taskList, std::string_view title = "") const;
    
    // Task management with enhanced error handling
    [[nodiscard]] TaskResult addTask(std::string_view name, TaskStatus status = TaskStatus::TODO, 
                                     TaskPriority priority = TaskPriority::LOW);
    [[nodiscard]] TaskResult addTask(std::string_view name, std::string_view description,
                                     TaskStatus status = TaskStatus::TODO, TaskPriority priority = TaskPriority::LOW,
                                     const std::optional<std::chrono::system_clock::time_point>& dueDate = std::nullopt,
                                     const std::vector<std::string>& tags = {});
    [[nodiscard]] TaskResult removeTask(int id);
    [[nodiscard]] TaskResult updateTask(int id, std::string_view name, TaskStatus status, TaskPriority priority);
    
    // Task retrieval
    [[nodiscard]] Task* findTask(int id) noexcept;
    [[nodiscard]] const Task* findTask(int id) const noexcept;
    [[nodiscard]] std::vector<Task*> searchTasks(std::string_view query) const;
    [[nodiscard]] std::vector<Task*> advancedSearch(std::string_view query) const; // Phase 2: Advanced search using index
    [[nodiscard]] std::vector<Task*> getTasksByStatus(TaskStatus status) const;
    [[nodiscard]] std::vector<Task*> getTasksByPriority(TaskPriority priority) const;
    [[nodiscard]] std::vector<Task*> getTasksByTag(std::string_view tag) const;
    [[nodiscard]] std::vector<Task*> getOverdueTasks() const;
    
    // Statistics
    [[nodiscard]] TaskStats getStatistics() const;
    
    // Data persistence
    void save() const;
    
    // Display methods with enhanced formatting
    void showAllTasks() const;
    void showFilteredTasks(TaskStatus status) const;
    void showFilteredTasks(TaskPriority priority) const;
    void showTaskDetails(int id) const;
    void showOverdueTasks() const;
    void showStatistics() const;
    
    // Utility methods
    [[nodiscard]] int getNextId() const noexcept;
    [[nodiscard]] bool isEmpty() const noexcept;
    [[nodiscard]] size_t size() const noexcept;
};

#endif // TASKS_HPP
