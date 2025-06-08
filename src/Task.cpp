#include "Task.hpp"
#include "utils.hpp"
#include <sstream>
#include <iomanip>
#include <vector>

Task::Task(int id, const std::string& name, int status, int priority)
    : id(id), name(name), status(status), priority(priority) {}

int Task::getId() const {
    return id;
}

std::string Task::getName() const {
    return name;
}

int Task::getStatus() const {
    return status;
}

int Task::getPriority() const {
    return priority;
}

void Task::setName(const std::string& name) {
    this->name = name;
}

void Task::setStatus(int status) {
    this->status = status;
}

void Task::setPriority(int priority) {
    this->priority = priority;
}

std::string Task::getStatusString() const {
    switch (status) {
        case 1: return "To-Do";
        case 2: return "In Progress";
        case 3: return "Completed";
        default: return "Unknown";
    }
}

std::string Task::getPriorityString() const {
    switch (priority) {
        case 1: return "Low";
        case 2: return "Medium";
        case 3: return "High";
        default: return "Unknown";
    }
}

nlohmann::json Task::toJson() const {
    return nlohmann::json{
        {"id", id},
        {"name", name},
        {"status", status},
        {"priority", priority}
    };
}

Task Task::fromJson(const nlohmann::json& j) {
    return Task(
        j.at("id").get<int>(),
        j.at("name").get<std::string>(),
        j.at("status").get<int>(),
        j.at("priority").get<int>()
    );
}

std::string Task::toString() const {
    std::ostringstream oss;
    
    // Color coding based on status and priority
    std::string statusColor = Utils::YELLOW; // Default
    std::string priorityColor = Utils::CYAN; // Default
    
    switch (status) {
        case 1: statusColor = Utils::RED; break;     // To-Do
        case 2: statusColor = Utils::YELLOW; break;  // In Progress
        case 3: statusColor = Utils::GREEN; break;   // Completed
    }
    
    switch (priority) {
        case 1: priorityColor = Utils::BLUE; break;    // Low
        case 2: priorityColor = Utils::YELLOW; break;  // Medium
        case 3: priorityColor = Utils::RED; break;     // High
    }
    
    oss << std::left << std::setw(4) << id
        << std::setw(25) << name
        << statusColor << std::setw(12) << getStatusString() << Utils::RESET
        << priorityColor << std::setw(8) << getPriorityString() << Utils::RESET;
    
    return oss.str();
}
