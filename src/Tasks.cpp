#include "Tasks.hpp"
#include "TaskSearchIndex.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ranges>
#include <format>
#include <execution>
#include <future>

// Constructor: Initialize task manager with data file path and load existing tasks
Tasks::Tasks(std::filesystem::path dataFile) : nextId(1), dataFile(std::move(dataFile)) {
    loadFromFile();
}

// Add a basic task with minimal information (name, status, priority)
TaskResult Tasks::addTask(std::string_view name, TaskStatus status, TaskPriority priority) {
    try {
        // Create new task with auto-incremented ID
        auto task = std::make_unique<Task>(nextId++, name, status, priority);
        tasks.push_back(std::move(task));

        // Mark cached data as outdated for lazy recomputation
        index_dirty_ = true; // Mark search index as dirty - Phase 2 optimization
        stats_dirty_ = true; // Mark statistics as dirty - Phase 2 optimization

        // Persist changes to file immediately
        saveToFile();
        return TaskResult::successResult("Task added successfully!");
    }
    catch (const std::exception& e) {
        --nextId; // Rollback ID on failure to maintain consistency
        return TaskResult::errorResult(std::format("Failed to add task: {}", e.what()));
    }
}

// Add a comprehensive task with all possible fields (description, due date, tags)
TaskResult Tasks::addTask(std::string_view name, std::string_view description,
    TaskStatus status, TaskPriority priority,
    const std::optional<std::chrono::system_clock::time_point>& dueDate,
    const std::vector<std::string>& tags) {
    try {
        // Create base task structure
        auto task = std::make_unique<Task>(nextId++, name, status, priority);

        // Add optional fields if provided
        task->setDescription(description);
        if (dueDate) {
            task->setDueDate(dueDate);
        }

        // Add all tags to the task
        for (const auto& tag : tags) {
            task->addTag(tag);
        }

        tasks.push_back(std::move(task));

        // Invalidate cached data for consistency
        index_dirty_ = true; // Mark search index as dirty - Phase 2 optimization
        stats_dirty_ = true; // Mark statistics as dirty - Phase 2 optimization

        saveToFile();
        return TaskResult::successResult("Task added successfully!");
    }
    catch (const std::exception& e) {
        --nextId; // Rollback ID on failure
        return TaskResult::errorResult(std::format("Failed to add task: {}", e.what()));
    }
}

// Remove a specific task by ID
TaskResult Tasks::removeTask(int id) {
    // Use modern C++20 ranges to find task efficiently
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    if (it != tasks.end()) {
        tasks.erase(it);

        // Update cached data flags
        index_dirty_ = true; // Mark search index as dirty - Phase 2 optimization
        stats_dirty_ = true; // Mark statistics as dirty - Phase 2 optimization

        saveToFile();
        return TaskResult::successResult("Task removed successfully!");
    }

    return TaskResult::errorResult(std::format("Task with ID {} not found!", id));
}

// Clear all tasks from the system
TaskResult Tasks::removeAllTasks() {
    if (tasks.empty()) {
        return TaskResult::errorResult("No tasks to remove!");
    }

    // Store count for user feedback before clearing
    size_t removedCount = tasks.size();
    tasks.clear();

    // Invalidate all cached data
    index_dirty_ = true; // Mark search index as dirty
    stats_dirty_ = true; // Mark statistics as dirty

    saveToFile();

    return TaskResult::successResult(std::format("All {} tasks removed successfully!", removedCount));
}

// Update an existing task's basic properties
TaskResult Tasks::updateTask(int id, std::string_view name, TaskStatus status, TaskPriority priority) {
    if (auto task = findTask(id)) {
        try {
            // Update all modifiable fields
            task->setName(name);
            task->setStatus(status);
            task->setPriority(priority);

            // Mark cached data as stale
            index_dirty_ = true; // Mark search index as dirty - Phase 2 optimization
            stats_dirty_ = true; // Mark statistics as dirty - Phase 2 optimization

            saveToFile();
            return TaskResult::successResult("Task updated successfully!");
        }
        catch (const std::exception& e) {
            return TaskResult::errorResult(std::format("Failed to update task: {}", e.what()));
        }
    }

    return TaskResult::errorResult(std::format("Task with ID {} not found!", id));
}

// Find a task by ID (mutable version for modification)
Task* Tasks::findTask(int id) noexcept {
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    return (it != tasks.end()) ? it->get() : nullptr;
}

// Basic text search through all tasks - simple string matching
std::vector<Task*> Tasks::searchTasks(std::string_view query) const {
    std::vector<Task*> results;
    results.reserve(std::min(tasks.size(), size_t{ 100 })); // Reserve reasonable capacity - Phase 1 optimization

    // Linear search through all tasks for matching text
    for (const auto& task : tasks) {
        if (task->matches(query)) {
            results.push_back(task.get());
        }
    }

    return results;
}

// Filter tasks by their current status (TODO, IN_PROGRESS, COMPLETED)
std::vector<Task*> Tasks::getTasksByStatus(TaskStatus status) const {
    std::vector<Task*> results;
    results.reserve(tasks.size() / 3); // Assume roughly 1/3 of tasks match any given status

    // Use C++20 ranges for functional-style filtering
    for (const auto& task : tasks | std::views::filter([status](const auto& t) {
        return t->getStatus() == status;
        })) {
        results.push_back(task.get());
    }

    return results;
}

// Filter tasks by their priority level (LOW, MEDIUM, HIGH)
std::vector<Task*> Tasks::getTasksByPriority(TaskPriority priority) const {
    std::vector<Task*> results;

    // Modern C++20 ranges-based filtering for cleaner code
    for (const auto& task : tasks | std::views::filter([priority](const auto& t) {
        return t->getPriority() == priority;
        })) {
        results.push_back(task.get());
    }

    return results;
}

// Find all tasks that contain a specific tag
std::vector<Task*> Tasks::getTasksByTag(std::string_view tag) const {
    std::vector<Task*> results;

    // Filter by tag presence using task's hasTag method
    for (const auto& task : tasks | std::views::filter([tag](const auto& t) {
        return t->hasTag(tag);
        })) {
        results.push_back(task.get());
    }

    return results;
}

// Get all tasks that have passed their due date
std::vector<Task*> Tasks::getOverdueTasks() const {
    std::vector<Task*> results;

    // Filter tasks where due date has passed current time
    for (const auto& task : tasks | std::views::filter([](const auto& t) {
        return t->isOverdue();
        })) {
        results.push_back(task.get());
    }

    return results;
}

// Compute and cache task statistics for performance optimization
TaskStats Tasks::getStatistics() const {
    // Lazy evaluation of statistics - return cached results if available
    if (!stats_dirty_ && cached_stats_) {
        return *cached_stats_;
    }

    TaskStats stats{};

    // Single-pass computation through all tasks for efficiency
    for (const auto& task : tasks) {
        ++stats.total;

        // Count tasks by status
        switch (task->getStatus()) {
        case TaskStatus::TODO: ++stats.todo; break;
        case TaskStatus::IN_PROGRESS: ++stats.inProgress; break;
        case TaskStatus::COMPLETED: ++stats.completed; break;
        }

        // Count tasks by priority
        switch (task->getPriority()) {
        case TaskPriority::LOW: ++stats.lowPriority; break;
        case TaskPriority::MEDIUM: ++stats.mediumPriority; break;
        case TaskPriority::HIGH: ++stats.highPriority; break;
        }

        // Count overdue tasks
        if (task->isOverdue()) {
            ++stats.overdue;
        }
    }

    // Cache the computed results for subsequent calls
    cached_stats_ = stats;
    stats_dirty_ = false;

    return stats;
}

// Simple wrapper for file saving
void Tasks::save() const {
    saveToFile();
}

// Display all tasks in a formatted table
void Tasks::showAllTasks() const {
    if (tasks.empty()) {
        std::cout << Utils::YELLOW << "No tasks found!" << Utils::RESET << std::endl;
        return;
    }

    // Sort tasks for display
    auto sortedTasks = getSortedTasks();

    // Column widths for better alignment
    const int idWidth = 4;
    const int nameWidth = 35;
    const int statusWidth = 12;
    const int priorityWidth = 10;
    const int dueDateWidth = 15;

    // Table header with ASCII borders for better compatibility
    printTableHeader(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    // Data rows
    for (const auto& task : sortedTasks) {
        printTaskRow(task, idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);
    }

    // Table footer
    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    std::cout << Utils::CYAN << "📋 Total tasks: " << tasks.size() << Utils::RESET << std::endl;
}

// Display detailed information for a specific task
void Tasks::showTaskDetails(int id) const {
    if (auto task = findTask(id)) {
        std::cout << task->toDetailedString() << std::endl;
    }
    else {
        std::cout << Utils::RED << "Task with ID " << id << " not found!" << Utils::RESET << std::endl;
    }
}

// Display tasks filtered by status with formatted output
void Tasks::showFilteredTasks(TaskStatus status) const {
    auto filteredTasks = getTasksByStatus(status);

    if (filteredTasks.empty()) {
        // Create a temporary task just to get the status string
        Task temp(0, "temp", status, TaskPriority::LOW);
        std::cout << Utils::YELLOW << "No tasks found with status: "
            << temp.getStatusString() << Utils::RESET << std::endl;
        return;
    }

    // Create a temporary task just to get the status string
    Task temp(0, "temp", status, TaskPriority::LOW);
    displayTaskList(filteredTasks, std::format("Tasks with status: {}", temp.getStatusString()));
}

// Display tasks filtered by priority with formatted output
void Tasks::showFilteredTasks(TaskPriority priority) const {
    auto filteredTasks = getTasksByPriority(priority);
    if (filteredTasks.empty()) {
        // Create a temporary task just to get the priority string
        Task temp(0, "temp", TaskStatus::TODO, priority);
        std::cout << Utils::YELLOW << "No tasks found with priority: "
            << temp.getPriorityString() << Utils::RESET << std::endl;
        return;
    }

    // Create a temporary task just to get the priority string
    Task temp(0, "temp", TaskStatus::TODO, priority);
    displayTaskList(filteredTasks, std::format("Tasks with priority: {}", temp.getPriorityString()));
}

// Display all overdue tasks in a formatted list
void Tasks::showOverdueTasks() const {
    auto overdueTasks = getOverdueTasks();

    if (overdueTasks.empty()) {
        std::cout << Utils::YELLOW << "No overdue tasks found!" << Utils::RESET << std::endl;
        return;
    }

    displayTaskList(overdueTasks, "Overdue Tasks");
}

// Display task statistics in a formatted table
void Tasks::showStatistics() const {
    auto stats = getStatistics();

    std::cout << Utils::BOLD << "[STATS] Task Statistics" << Utils::RESET << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << std::endl;

    // Create a two-column table: Status on left, Priority on right
    const int leftColWidth = 25;
    const int rightColWidth = 25;

    // Table header
    std::cout << "+" << std::string(leftColWidth, '-');
    std::cout << "+" << std::string(rightColWidth, '-') << "+" << std::endl;

    std::cout << "|" << Utils::BOLD << " Task Status" << Utils::RESET;
    std::cout << std::string(leftColWidth - 12, ' ') << "|";
    std::cout << Utils::BOLD << " Priority Breakdown" << Utils::RESET;
    std::cout << std::string(rightColWidth - 19, ' ') << "|" << std::endl;

    // Header separator
    std::cout << "+" << std::string(leftColWidth, '-');
    std::cout << "+" << std::string(rightColWidth, '-') << "+" << std::endl;

    // Data rows
    // Row 1: To-Do | High priority  
    std::cout << "| To-Do: " << Utils::RED << stats.todo << Utils::RESET;
    std::cout << std::string(leftColWidth - 8 - std::to_string(stats.todo).length(), ' ') << "|";
    std::cout << " High: " << Utils::RED << stats.highPriority << Utils::RESET;
    std::cout << std::string(rightColWidth - 7 - std::to_string(stats.highPriority).length(), ' ') << "|" << std::endl;

    // Row 2: In Progress | Medium priority
    std::cout << "| In Progress: " << Utils::YELLOW << stats.inProgress << Utils::RESET;
    std::cout << std::string(leftColWidth - 14 - std::to_string(stats.inProgress).length(), ' ') << "|";
    std::cout << " Medium: " << Utils::YELLOW << stats.mediumPriority << Utils::RESET;
    std::cout << std::string(rightColWidth - 9 - std::to_string(stats.mediumPriority).length(), ' ') << "|" << std::endl;

    // Row 3: Completed | Low priority
    std::cout << "| Completed: " << Utils::GREEN << stats.completed << Utils::RESET;
    std::cout << std::string(leftColWidth - 12 - std::to_string(stats.completed).length(), ' ') << "|";
    std::cout << " Low: " << Utils::BLUE << stats.lowPriority << Utils::RESET;
    std::cout << std::string(rightColWidth - 6 - std::to_string(stats.lowPriority).length(), ' ') << "|" << std::endl;

    // Table footer
    std::cout << "+" << std::string(leftColWidth, '-');
    std::cout << "+" << std::string(rightColWidth, '-') << "+" << std::endl;

    // Total tasks summary
    std::cout << std::endl;
    std::cout << Utils::CYAN << "📋 Total tasks: " << stats.total << Utils::RESET << std::endl;

    // Overdue tasks warning (if any)
    if (stats.overdue > 0) {
        std::cout << Utils::RED << "⚠️  Overdue tasks: " << stats.overdue << Utils::RESET << std::endl;
    }
}

// Load tasks from JSON file on disk
void Tasks::loadFromFile() {
    // Create directory structure if data file doesn't exist
    if (!std::filesystem::exists(dataFile)) {
        std::filesystem::create_directories(dataFile.parent_path());
        return;
    }

    try {
        std::ifstream file(dataFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open data file for reading");
        }

        nlohmann::json j;
        file >> j;

        // Restore next ID counter if present
        if (j.contains("nextId")) {
            nextId = j["nextId"];
        }

        // Load all tasks from JSON array
        if (j.contains("tasks")) {
            for (const auto& taskJson : j["tasks"]) {
                tasks.push_back(std::make_unique<Task>(Task::fromJson(taskJson)));
            }
        }
    }
    catch (const std::exception& e) {
        std::cout << Utils::RED << "Error loading data: " << e.what() << Utils::RESET << std::endl;
    }
}

// Save all tasks to JSON file on disk
void Tasks::saveToFile() const {
    try {
        // Ensure directory exists
        std::filesystem::create_directories(dataFile.parent_path());

        // Build JSON structure with metadata and task array
        nlohmann::json j{
            {"nextId", nextId},
            {"tasks", nlohmann::json::array()}
        };

        // Convert all tasks to JSON format
        for (const auto& task : tasks) {
            j["tasks"].push_back(task->toJson());
        }

        std::ofstream file(dataFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open data file for writing");
        }

        // Write with 4-space indentation for readability
        file << j.dump(4);
    }
    catch (const std::exception& e) {
        std::cout << Utils::RED << "Error saving data: " << e.what() << Utils::RESET << std::endl;
    }
}

// Helper method to get tasks sorted by priority and status
std::vector<Task*> Tasks::getSortedTasks() const {
    std::vector<Task*> sortedTasks;

    // Convert unique_ptr to raw pointers for sorting
    for (const auto& task : tasks) {
        sortedTasks.push_back(task.get());
    }

    // Sort using Task's operator< for consistent ordering
    std::ranges::sort(sortedTasks, [](const Task* a, const Task* b) {
        return *a < *b;  // Use Task's operator<
        });

    return sortedTasks;
}

// Helper method to display a list of tasks with a title
void Tasks::displayTaskList(const std::vector<Task*>& taskList, std::string_view title) const {
    if (!title.empty()) {
        std::cout << Utils::BOLD << title << Utils::RESET << std::endl;
        std::cout << std::endl;
    }

    // Column widths for better alignment
    const int idWidth = 4;
    const int nameWidth = 35;
    const int statusWidth = 12;
    const int priorityWidth = 10;
    const int dueDateWidth = 15;

    // Table header with ASCII borders for better compatibility
    printTableHeader(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    // Data rows
    for (const auto& task : taskList) {
        printTaskRow(task, idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);
    }

    // Table footer
    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    std::cout << Utils::CYAN << "📊 Count: " << taskList.size() << Utils::RESET << std::endl;
}

// Const version of findTask for read-only operations
const Task* Tasks::findTask(int id) const noexcept {
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    return (it != tasks.end()) ? it->get() : nullptr;
}

// Getter methods for task manager state
[[nodiscard]] int Tasks::getNextId() const noexcept {
    return nextId;
}

[[nodiscard]] bool Tasks::isEmpty() const noexcept {
    return tasks.empty();
}

[[nodiscard]] size_t Tasks::size() const noexcept {
    return tasks.size();
}

// Phase 2 optimization: Search index implementation
void Tasks::rebuildSearchIndex() const {
    if (!index_dirty_) return;

    search_index_.clear();

    // Add all tasks to the search index for faster searching
    for (const auto& task : tasks) {
        search_index_.addTask(*task);
    }

    index_dirty_ = false;
}

// Advanced search using the search index for better performance
std::vector<Task*> Tasks::advancedSearch(std::string_view query) const {
    if (query.empty()) {
        return {};
    }

    // Rebuild index if necessary
    rebuildSearchIndex();

    // Use prefix search for better performance
    auto taskRefs = search_index_.searchPrefix(query);

    std::vector<Task*> results;
    results.reserve(taskRefs.size());

    for (const auto& taskRef : taskRefs) {
        // Find the corresponding Task* in our tasks vector
        const Task& task = taskRef.get();
        auto it = std::ranges::find_if(tasks, [&task](const auto& taskPtr) {
            return taskPtr.get() == &task;
            });

        if (it != tasks.end()) {
            results.push_back(it->get());
        }
    }

    // Remove duplicates and sort by relevance (task ID for now)
    std::ranges::sort(results, [](const Task* a, const Task* b) {
        return a->getId() < b->getId();
        });

    results.erase(std::unique(results.begin(), results.end()), results.end());

    return results;
}

// Phase 2 optimization: Async file I/O implementation
void Tasks::saveAsync() const {
    // Prevent concurrent save operations by checking if one is already in progress
    if (save_future_.valid() &&
        save_future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return;
    }

    // Create deep copies of all tasks for thread-safe async operation
    // This prevents data races when the main thread modifies tasks while saving
    std::vector<std::unique_ptr<Task>> task_copy;
    task_copy.reserve(tasks.size());

    for (const auto& task : tasks) {
        // Create deep copy for async operation
        auto task_copy_ptr = std::make_unique<Task>(*task);
        task_copy.push_back(std::move(task_copy_ptr));
    }

    std::filesystem::path file_path = dataFile;

    // Launch async save operation - tasks are saved in background thread
    save_future_ = std::async(std::launch::async, [task_copy = std::move(task_copy), file_path]() {
        try {
            // Ensure parent directory exists before writing
            std::filesystem::create_directories(file_path.parent_path());

            // Build JSON structure with task data and metadata
            nlohmann::json j{
                {"nextId", 1}, // This would need to be passed properly in a real implementation
                {"tasks", nlohmann::json::array()}
            };

            // Convert all tasks to JSON format
            for (const auto& task : task_copy) {
                j["tasks"].push_back(task->toJson());
            }

            // Write to file with proper formatting
            std::ofstream file(file_path);
            if (!file.is_open()) {
                std::cerr << "Error: Cannot open file for writing: " << file_path << std::endl;
                return;
            }

            // Write with 4-space indentation for readability
            file << j.dump(4);
        }
        catch (const std::exception& e) {
            std::cerr << "Error saving tasks asynchronously: " << e.what() << std::endl;
        }
        });
}

// Block execution until any pending save operation completes
void Tasks::waitForSave() const {
    if (save_future_.valid()) {
        save_future_.wait();
    }
}

/*
 * ========================
 * Table Display Utilities
 * ========================
 */

 // Print horizontal separator line with proper column alignment
void Tasks::printTableSeparator(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    std::cout << "+-" << std::string(idWidth, '-');      // ID column border
    std::cout << "-+-" << std::string(nameWidth, '-');   // Name column border  
    std::cout << "-+-" << std::string(statusWidth, '-'); // Status column border
    std::cout << "-+-" << std::string(priorityWidth, '-'); // Priority column border
    std::cout << "-+-" << std::string(dueDateWidth, '-'); // Due date column border
    std::cout << "-+" << std::endl;
}

// Print table header with column titles and proper formatting
void Tasks::printTableHeader(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    // Top border
    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    // Header row with bold formatting and proper column alignment
    std::cout << "| " << Utils::BOLD << std::left << std::setw(idWidth) << "ID";
    std::cout << " | " << std::left << std::setw(nameWidth) << "Task Name";
    std::cout << " | " << std::left << std::setw(statusWidth) << "Status";
    std::cout << " | " << std::left << std::setw(priorityWidth) << "Priority";
    std::cout << " | " << std::left << std::setw(dueDateWidth) << "Due Date";
    std::cout << " |" << Utils::RESET << std::endl;

    // Header separator
    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);
}

// Format task name with truncation and overdue indicator
std::string Tasks::formatTaskName(const Task* task, int maxWidth) const {
    std::string taskName = task->getName();

    // Truncate name if it exceeds column width, leaving space for "..."
    std::string displayName = taskName.length() > static_cast<size_t>(maxWidth) ?
        taskName.substr(0, maxWidth - 3) + "..." : taskName;

    // Add visual overdue indicator [!]
    if (task->isOverdue()) {
        displayName += " [!]";
        // Re-check length after adding indicator and truncate if needed
        if (displayName.length() > static_cast<size_t>(maxWidth)) {
            displayName = displayName.substr(0, maxWidth - 3) + "...";
        }
    }

    return displayName;
}

// Print a single task row with proper formatting and colors
void Tasks::printTaskRow(const Task* task, int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    // Prepare formatted data for each column
    std::string displayName = formatTaskName(task, nameWidth);
    std::string statusStr = task->getStatusString();
    std::string priorityStr = task->getPriorityString();
    std::string dueDateStr = task->getDueDate() ? Utils::formatDate(*task->getDueDate()) : "";

    // Get appropriate colors for status and priority visualization
    std::string_view statusColor = Utils::getStatusColor(task->getStatus());
    std::string_view priorityColor = Utils::getPriorityColor(task->getPriority());

    // Display row with proper alignment and colors
    std::cout << "| " << std::left << std::setw(idWidth) << task->getId();
    std::cout << " | " << std::left << std::setw(nameWidth) << displayName;
    std::cout << " | " << statusColor << std::left << std::setw(statusWidth) << statusStr << Utils::RESET;
    std::cout << " | " << priorityColor << std::left << std::setw(priorityWidth) << priorityStr << Utils::RESET;
    std::cout << " | " << std::left << std::setw(dueDateWidth) << dueDateStr;
    std::cout << " |" << std::endl;
}
