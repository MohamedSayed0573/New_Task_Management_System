#include "Tasks.hpp"
#include "TaskSearchIndex.hpp"
#include "benchmark.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ranges>
#include <format>
#include <execution>
#include <future>

Tasks::Tasks(std::filesystem::path dataFile) : nextId(1), dataFile(std::move(dataFile)) {
    loadFromFile();
}

TaskResult Tasks::addTask(std::string_view name, TaskStatus status, TaskPriority priority) {
    try {
        auto task = std::make_unique<Task>(nextId++, name, status, priority);
        tasks.push_back(std::move(task));
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

TaskResult Tasks::addTask(std::string_view name, std::string_view description,
    TaskStatus status, TaskPriority priority,
    const std::optional<std::chrono::system_clock::time_point>& dueDate,
    const std::vector<std::string>& tags) {
    try {
        auto task = std::make_unique<Task>(nextId++, name, status, priority);
        task->setDescription(description);
        if (dueDate) {
            task->setDueDate(dueDate);
        }
        for (const auto& tag : tags) {
            task->addTag(tag);
        }

        tasks.push_back(std::move(task));
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

TaskResult Tasks::removeTask(int id) {
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    if (it != tasks.end()) {
        tasks.erase(it);
        index_dirty_ = true; // Mark search index as dirty - Phase 2 optimization
        stats_dirty_ = true; // Mark statistics as dirty - Phase 2 optimization
        saveToFile();
        return TaskResult::successResult("Task removed successfully!");
    }

    return TaskResult::errorResult(std::format("Task with ID {} not found!", id));
}

TaskResult Tasks::updateTask(int id, std::string_view name, TaskStatus status, TaskPriority priority) {
    if (auto task = findTask(id)) {
        try {
            task->setName(name);
            task->setStatus(status);
            task->setPriority(priority);
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

Task* Tasks::findTask(int id) noexcept {
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    return (it != tasks.end()) ? it->get() : nullptr;
}

std::vector<Task*> Tasks::searchTasks(std::string_view query) const {
    std::vector<Task*> results;
    results.reserve(std::min(tasks.size(), size_t{ 100 })); // Reserve reasonable capacity - Phase 1 optimization

    for (const auto& task : tasks) {
        if (task->matches(query)) {
            results.push_back(task.get());
        }
    }

    return results;
}

std::vector<Task*> Tasks::getTasksByStatus(TaskStatus status) const {
    BENCHMARK("Get Tasks By Status");

    std::vector<Task*> results;
    results.reserve(tasks.size() / 3); // Assume roughly 1/3 of tasks match any given status

    for (const auto& task : tasks | std::views::filter([status](const auto& t) {
        return t->getStatus() == status;
        })) {
        results.push_back(task.get());
    }

    return results;
}

std::vector<Task*> Tasks::getTasksByPriority(TaskPriority priority) const {
    std::vector<Task*> results;

    for (const auto& task : tasks | std::views::filter([priority](const auto& t) {
        return t->getPriority() == priority;
        })) {
        results.push_back(task.get());
    }

    return results;
}

std::vector<Task*> Tasks::getTasksByTag(std::string_view tag) const {
    std::vector<Task*> results;

    for (const auto& task : tasks | std::views::filter([tag](const auto& t) {
        return t->hasTag(tag);
        })) {
        results.push_back(task.get());
    }

    return results;
}

std::vector<Task*> Tasks::getOverdueTasks() const {
    std::vector<Task*> results;

    for (const auto& task : tasks | std::views::filter([](const auto& t) {
        return t->isOverdue();
        })) {
        results.push_back(task.get());
    }

    return results;
}

TaskStats Tasks::getStatistics() const {
    BENCHMARK("Statistics Computation");

    // Lazy evaluation of statistics
    if (!stats_dirty_ && cached_stats_) {
        return *cached_stats_;
    }

    TaskStats stats{};

    // Simple single-pass computation
    for (const auto& task : tasks) {
        ++stats.total;

        switch (task->getStatus()) {
        case TaskStatus::TODO: ++stats.todo; break;
        case TaskStatus::IN_PROGRESS: ++stats.inProgress; break;
        case TaskStatus::COMPLETED: ++stats.completed; break;
        }

        switch (task->getPriority()) {
        case TaskPriority::LOW: ++stats.lowPriority; break;
        case TaskPriority::MEDIUM: ++stats.mediumPriority; break;
        case TaskPriority::HIGH: ++stats.highPriority; break;
        }

        if (task->isOverdue()) {
            ++stats.overdue;
        }
    }

    // Cache the results
    cached_stats_ = stats;
    stats_dirty_ = false;

    return stats;
}

void Tasks::save() const {
    saveToFile();
}

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

void Tasks::showTaskDetails(int id) const {
    if (auto task = findTask(id)) {
        std::cout << task->toDetailedString() << std::endl;
    }
    else {
        std::cout << Utils::RED << "Task with ID " << id << " not found!" << Utils::RESET << std::endl;
    }
}

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

void Tasks::showFilteredTasks(TaskPriority priority) const {
    auto filteredTasks = getTasksByPriority(priority);
    if (filteredTasks.empty()) {
        std::cout << Utils::YELLOW << "No tasks found with priority: "
            << Utils::getPriorityString(priority) << Utils::RESET << std::endl;
        return;
    }

    displayTaskList(filteredTasks, std::format("Tasks with priority: {}", Utils::getPriorityString(priority)));
}

void Tasks::showOverdueTasks() const {
    auto overdueTasks = getOverdueTasks();

    if (overdueTasks.empty()) {
        std::cout << Utils::YELLOW << "No overdue tasks found!" << Utils::RESET << std::endl;
        return;
    }

    displayTaskList(overdueTasks, "Overdue Tasks");
}

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

void Tasks::loadFromFile() {
    if (!std::filesystem::exists(dataFile)) {
        // Create directory if it doesn't exist
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

        if (j.contains("nextId")) {
            nextId = j["nextId"];
        }

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

void Tasks::saveToFile() const {
    try {
        // Ensure directory exists
        std::filesystem::create_directories(dataFile.parent_path());

        nlohmann::json j{
            {"nextId", nextId},
            {"tasks", nlohmann::json::array()}
        };

        for (const auto& task : tasks) {
            j["tasks"].push_back(task->toJson());
        }

        std::ofstream file(dataFile);
        if (!file.is_open()) {
            throw std::runtime_error("Could not open data file for writing");
        }

        file << j.dump(4);
    }
    catch (const std::exception& e) {
        std::cout << Utils::RED << "Error saving data: " << e.what() << Utils::RESET << std::endl;
    }
}

std::vector<Task*> Tasks::getSortedTasks() const {
    std::vector<Task*> sortedTasks;

    for (const auto& task : tasks) {
        sortedTasks.push_back(task.get());
    }

    std::ranges::sort(sortedTasks, [](const Task* a, const Task* b) {
        return *a < *b;  // Use Task's operator<
        });

    return sortedTasks;
}

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

// Const version of findTask
const Task* Tasks::findTask(int id) const noexcept {
    auto it = std::ranges::find_if(tasks, [id](const auto& task) {
        return task->getId() == id;
        });

    return (it != tasks.end()) ? it->get() : nullptr;
}

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

    BENCHMARK("Search Index Rebuild");
    search_index_.clear();

    for (const auto& task : tasks) {
        search_index_.addTask(*task);
    }

    index_dirty_ = false;
}

std::vector<Task*> Tasks::advancedSearch(std::string_view query) const {
    BENCHMARK("Advanced Search");

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
    // Don't start new save if one is in progress
    if (save_future_.valid() &&
        save_future_.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
        return;
    }

    // Create a copy of tasks for async saving
    std::vector<std::unique_ptr<Task>> task_copy;
    task_copy.reserve(tasks.size());

    for (const auto& task : tasks) {
        // Create deep copy for async operation
        auto task_copy_ptr = std::make_unique<Task>(*task);
        task_copy.push_back(std::move(task_copy_ptr));
    }

    std::filesystem::path file_path = dataFile;

    save_future_ = std::async(std::launch::async, [task_copy = std::move(task_copy), file_path]() {
        BENCHMARK("Async File Save");

        try {
            nlohmann::json jsonArray = nlohmann::json::array();

            for (const auto& task : task_copy) {
                jsonArray.push_back(task->toJson());
            }

            std::ofstream outFile(file_path);
            if (!outFile.is_open()) {
                std::cerr << "Error: Cannot open file for writing: " << file_path << std::endl;
                return;
            }

            outFile << jsonArray.dump(2);
            outFile.close();

        }
        catch (const std::exception& e) {
            std::cerr << "Error saving tasks asynchronously: " << e.what() << std::endl;
        }
        });
}

void Tasks::waitForSave() const {
    if (save_future_.valid()) {
        save_future_.wait();
    }
}

// Table display helper methods
void Tasks::printTableSeparator(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    std::cout << "+-" << std::string(idWidth, '-');
    std::cout << "-+-" << std::string(nameWidth, '-');
    std::cout << "-+-" << std::string(statusWidth, '-');
    std::cout << "-+-" << std::string(priorityWidth, '-');
    std::cout << "-+-" << std::string(dueDateWidth, '-');
    std::cout << "-+" << std::endl;
}

void Tasks::printTableHeader(int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);

    std::cout << "| " << Utils::BOLD << std::left << std::setw(idWidth) << "ID";
    std::cout << " | " << std::left << std::setw(nameWidth) << "Task Name";
    std::cout << " | " << std::left << std::setw(statusWidth) << "Status";
    std::cout << " | " << std::left << std::setw(priorityWidth) << "Priority";
    std::cout << " | " << std::left << std::setw(dueDateWidth) << "Due Date";
    std::cout << " |" << Utils::RESET << std::endl;

    printTableSeparator(idWidth, nameWidth, statusWidth, priorityWidth, dueDateWidth);
}

std::string Tasks::formatTaskName(const Task* task, int maxWidth) const {
    std::string taskName = task->getName();
    std::string displayName = taskName.length() > static_cast<size_t>(maxWidth) ?
        taskName.substr(0, maxWidth - 3) + "..." : taskName;

    // Add overdue indicator
    if (task->isOverdue()) {
        displayName += " [!]";
        if (displayName.length() > static_cast<size_t>(maxWidth)) {
            displayName = displayName.substr(0, maxWidth - 3) + "...";
        }
    }

    return displayName;
}

void Tasks::printTaskRow(const Task* task, int idWidth, int nameWidth, int statusWidth, int priorityWidth, int dueDateWidth) const {
    std::string displayName = formatTaskName(task, nameWidth);
    std::string statusStr = task->getStatusString();
    std::string priorityStr = task->getPriorityString();
    std::string dueDateStr = task->getDueDate() ? Utils::formatDate(*task->getDueDate()) : "";

    // Get colors
    std::string_view statusColor = Utils::getStatusColor(task->getStatus());
    std::string_view priorityColor = Utils::getPriorityColor(task->getPriority());

    // Display the row
    std::cout << "| " << std::left << std::setw(idWidth) << task->getId();
    std::cout << " | " << std::left << std::setw(nameWidth) << displayName;
    std::cout << " | " << statusColor << std::left << std::setw(statusWidth) << statusStr << Utils::RESET;
    std::cout << " | " << priorityColor << std::left << std::setw(priorityWidth) << priorityStr << Utils::RESET;
    std::cout << " | " << std::left << std::setw(dueDateWidth) << dueDateStr;
    std::cout << " |" << std::endl;
}
