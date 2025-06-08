#include "Tasks.hpp"
#include "utils.hpp"
#include <fstream>
#include <iostream>
#include <algorithm>
#include <iomanip>

Tasks::Tasks(const std::string& dataFile) : nextId(1), dataFile(dataFile) {
    loadFromFile();
}

void Tasks::addTask(const std::string& name, int status, int priority) {
    if (name.empty()) {
        std::cout << Utils::RED << "Error: Task name cannot be empty!" << Utils::RESET << std::endl;
        return;
    }
    
    if (!Utils::isValidStatus(status)) {
        std::cout << Utils::RED << "Error: Invalid status! Use 1 (To-Do), 2 (In Progress), or 3 (Completed)" << Utils::RESET << std::endl;
        return;
    }
    
    if (!Utils::isValidPriority(priority)) {
        std::cout << Utils::RED << "Error: Invalid priority! Use 1 (Low), 2 (Medium), or 3 (High)" << Utils::RESET << std::endl;
        return;
    }
    
    tasks.push_back(std::make_unique<Task>(nextId++, name, status, priority));
    saveToFile();
    
    std::cout << Utils::GREEN << "Task added successfully!" << Utils::RESET << std::endl;
}

bool Tasks::removeTask(int id) {
    auto it = std::find_if(tasks.begin(), tasks.end(),
        [id](const std::unique_ptr<Task>& task) {
            return task->getId() == id;
        });
    
    if (it != tasks.end()) {
        tasks.erase(it);
        saveToFile();
        std::cout << Utils::GREEN << "Task removed successfully!" << Utils::RESET << std::endl;
        return true;
    }
    
    std::cout << Utils::RED << "Error: Task with ID " << id << " not found!" << Utils::RESET << std::endl;
    return false;
}

bool Tasks::updateTask(int id, const std::string& name, int status, int priority) {
    Task* task = findTask(id);
    if (!task) {
        std::cout << Utils::RED << "Error: Task with ID " << id << " not found!" << Utils::RESET << std::endl;
        return false;
    }
    
    if (name.empty()) {
        std::cout << Utils::RED << "Error: Task name cannot be empty!" << Utils::RESET << std::endl;
        return false;
    }
    
    if (!Utils::isValidStatus(status)) {
        std::cout << Utils::RED << "Error: Invalid status! Use 1 (To-Do), 2 (In Progress), or 3 (Completed)" << Utils::RESET << std::endl;
        return false;
    }
    
    if (!Utils::isValidPriority(priority)) {
        std::cout << Utils::RED << "Error: Invalid priority! Use 1 (Low), 2 (Medium), or 3 (High)" << Utils::RESET << std::endl;
        return false;
    }
    
    task->setName(name);
    task->setStatus(status);
    task->setPriority(priority);
    saveToFile();
    
    std::cout << Utils::GREEN << "Task updated successfully!" << Utils::RESET << std::endl;
    return true;
}

Task* Tasks::findTask(int id) {
    auto it = std::find_if(tasks.begin(), tasks.end(),
        [id](const std::unique_ptr<Task>& task) {
            return task->getId() == id;
        });
    
    return (it != tasks.end()) ? it->get() : nullptr;
}

std::vector<Task*> Tasks::searchTasks(const std::string& name) {
    std::vector<Task*> results;
    std::string searchLower = Utils::toLowerCase(name);
    
    for (const auto& task : tasks) {
        std::string taskNameLower = Utils::toLowerCase(task->getName());
        if (taskNameLower.find(searchLower) != std::string::npos) {
            results.push_back(task.get());
        }
    }
    
    return results;
}

void Tasks::showAllTasks() const {
    if (tasks.empty()) {
        std::cout << Utils::YELLOW << "No tasks found!" << Utils::RESET << std::endl;
        return;
    }
    
    Utils::printHeader();
    std::cout << Utils::BOLD << std::left 
              << std::setw(4) << "ID"
              << std::setw(25) << "Name"
              << std::setw(12) << "Status"
              << std::setw(8) << "Priority" << Utils::RESET << std::endl;
    Utils::printSeparator();
    
    for (const auto& task : tasks) {
        std::cout << task->toString() << std::endl;
    }
    
    Utils::printSeparator();
    std::cout << Utils::CYAN << "Total tasks: " << tasks.size() << Utils::RESET << std::endl;
}

void Tasks::showFilteredTasks(const std::string& filter) const {
    std::vector<Task*> filteredTasks;
    
    for (const auto& task : tasks) {
        bool match = false;
        
        if (filter == "-t" && task->getStatus() == 1) match = true;      // To-Do
        else if (filter == "-i" && task->getStatus() == 2) match = true; // In Progress
        else if (filter == "-c" && task->getStatus() == 3) match = true; // Completed
        else if (filter == "-l" && task->getPriority() == 1) match = true; // Low
        else if (filter == "-m" && task->getPriority() == 2) match = true; // Medium
        else if (filter == "-h" && task->getPriority() == 3) match = true; // High
        
        if (match) {
            filteredTasks.push_back(task.get());
        }
    }
    
    if (filteredTasks.empty()) {
        std::cout << Utils::YELLOW << "No tasks found matching the filter!" << Utils::RESET << std::endl;
        return;
    }
    
    Utils::printHeader();
    std::cout << Utils::BOLD << std::left 
              << std::setw(4) << "ID"
              << std::setw(25) << "Name"
              << std::setw(12) << "Status"
              << std::setw(8) << "Priority" << Utils::RESET << std::endl;
    Utils::printSeparator();
    
    for (const auto& task : filteredTasks) {
        std::cout << task->toString() << std::endl;
    }
    
    Utils::printSeparator();
    std::cout << Utils::CYAN << "Filtered tasks: " << filteredTasks.size() << Utils::RESET << std::endl;
}

void Tasks::loadFromFile() {
    std::ifstream file(dataFile);
    if (!file.is_open()) {
        // File doesn't exist yet, that's okay
        return;
    }
    
    try {
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
    
    file.close();
}

void Tasks::saveToFile() const {
    nlohmann::json j;
    j["nextId"] = nextId;
    
    for (const auto& task : tasks) {
        j["tasks"].push_back(task->toJson());
    }
    
    std::ofstream file(dataFile);
    if (file.is_open()) {
        file << j.dump(4);
        file.close();
    } else {
        std::cout << Utils::RED << "Error: Could not save to file!" << Utils::RESET << std::endl;
    }
}

int Tasks::getNextId() const {
    return nextId;
}

bool Tasks::isEmpty() const {
    return tasks.empty();
}

size_t Tasks::size() const {
    return tasks.size();
}
