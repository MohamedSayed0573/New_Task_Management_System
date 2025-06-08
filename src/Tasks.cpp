#include "Tasks.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>
#include <ranges>
#include <format>

Tasks::Tasks(std::filesystem::path dataFile) : nextId(1), dataFile(std::move(dataFile)) {
    loadFromFile();
}

TaskResult Tasks::addTask(std::string_view name, TaskStatus status, TaskPriority priority) {
    try {
        auto task = std::make_unique<Task>(nextId++, name, status, priority);
        tasks.push_back(std::move(task));
        saveToFile();
        return TaskResult::successResult("Task added successfully!");
    } catch (const std::exception& e) {
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
        saveToFile();
        return TaskResult::successResult("Task added successfully!");
    } catch (const std::exception& e) {
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
            saveToFile();
            return TaskResult::successResult("Task updated successfully!");
        } catch (const std::exception& e) {
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
    
    for (const auto& task : tasks) {
        if (task->matches(query)) {
            results.push_back(task.get());
        }
    }
    
    return results;
}

std::vector<Task*> Tasks::getTasksByStatus(TaskStatus status) const {
    std::vector<Task*> results;
    
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
    TaskStats stats{};
    
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
    
    return stats;
}

void Tasks::showAllTasks() const {
    if (tasks.empty()) {
        std::cout << Utils::YELLOW << "No tasks found!" << Utils::RESET << std::endl;
        return;
    }
    
    // Sort tasks for display
    auto sortedTasks = getSortedTasks();
    
    Utils::printHeader();
    std::cout << Utils::BOLD << std::left 
              << std::setw(4) << "ID"
              << std::setw(30) << "Name"
              << std::setw(12) << "Status"
              << std::setw(10) << "Priority"
              << std::setw(12) << "Due Date" << Utils::RESET << std::endl;
    Utils::printSeparator();
    
    for (const auto& task : sortedTasks) {
        std::cout << task->toString() << std::endl;
    }
    
    Utils::printSeparator();
    std::cout << Utils::CYAN << "Total tasks: " << tasks.size() << Utils::RESET << std::endl;
}

void Tasks::showTaskDetails(int id) const {
    if (auto task = findTask(id)) {
        std::cout << task->toDetailedString() << std::endl;
    } else {
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
    
    std::cout << Utils::BOLD << "📊 Task Statistics" << Utils::RESET << std::endl;
    std::cout << "==================" << std::endl;
    std::cout << "Total tasks: " << Utils::CYAN << stats.total << Utils::RESET << std::endl;
    std::cout << "To-Do: " << Utils::RED << stats.todo << Utils::RESET << std::endl;
    std::cout << "In Progress: " << Utils::YELLOW << stats.inProgress << Utils::RESET << std::endl;
    std::cout << "Completed: " << Utils::GREEN << stats.completed << Utils::RESET << std::endl;
    std::cout << std::endl;
    std::cout << "Priority breakdown:" << std::endl;
    std::cout << "High: " << Utils::RED << stats.highPriority << Utils::RESET << std::endl;
    std::cout << "Medium: " << Utils::YELLOW << stats.mediumPriority << Utils::RESET << std::endl;
    std::cout << "Low: " << Utils::BLUE << stats.lowPriority << Utils::RESET << std::endl;
    
    if (stats.overdue > 0) {
        std::cout << std::endl;
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
    } catch (const std::exception& e) {
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
    } catch (const std::exception& e) {
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
    Utils::printHeader();
    if (!title.empty()) {
        std::cout << Utils::BOLD << title << Utils::RESET << std::endl;
        Utils::printSeparator();
    }
    
    std::cout << Utils::BOLD << std::left 
              << std::setw(4) << "ID"
              << std::setw(30) << "Name"
              << std::setw(12) << "Status"
              << std::setw(10) << "Priority"
              << std::setw(12) << "Due Date" << Utils::RESET << std::endl;
    Utils::printSeparator();
    
    for (const auto& task : taskList) {
        std::cout << task->toString() << std::endl;
    }
    
    Utils::printSeparator();
    std::cout << Utils::CYAN << "Count: " << taskList.size() << Utils::RESET << std::endl;
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
