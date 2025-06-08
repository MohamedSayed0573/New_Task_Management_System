#include "Tasks.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <format>

class TodoApp {
private:
    Tasks tasks;
    
    // String constants for common error messages
    static constexpr const char* TASK_NOT_FOUND_MSG = "Task with ID {} not found!";
    static constexpr const char* INVALID_TASK_ID_MSG = "Invalid task ID: {}";
    static constexpr const char* USAGE_TAG_MSG = "Usage: tag <task_ID> <tag>";
    static constexpr const char* USAGE_UNTAG_MSG = "Usage: untag <task_ID> <tag>";
    static constexpr const char* USAGE_DUE_MSG = "Usage: due <task_ID> <date> (YYYY-MM-DD format)";
    static constexpr const char* USAGE_DETAIL_MSG = "Usage: detail <task_ID>";
    static constexpr const char* USAGE_REMOVE_MSG = "Usage: remove <task_ID>";
    static constexpr const char* TAG_ADDED_MSG = "Tag added successfully!";
    static constexpr const char* TAG_REMOVED_MSG = "Tag removed successfully!";
    static constexpr const char* DUE_DATE_SET_MSG = "Due date set successfully!";
    static constexpr const char* INVALID_DATE_FORMAT_MSG = "Invalid date format. Use YYYY-MM-DD";
    
    // Helper method to parse quoted arguments
    std::vector<std::string> parseArguments(int argc, char* argv[]) {
        std::vector<std::string> args;
        std::string current;
        bool inQuotes = false;
        
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            
            if (!inQuotes && (arg.front() == '"' || arg.front() == '\'')) {
                inQuotes = true;
                current = arg.substr(1);
                if (arg.back() == arg.front() && arg.length() > 1) {
                    current.pop_back();
                    args.push_back(current);
                    current.clear();
                    inQuotes = false;
                }
            } else if (inQuotes) {
                if (arg.back() == '"' || arg.back() == '\'') {
                    current += " " + arg.substr(0, arg.length() - 1);
                    args.push_back(current);
                    current.clear();
                    inQuotes = false;
                } else {
                    current += " " + arg;
                }
            } else {
                args.push_back(arg);
            }
        }
        
        if (inQuotes && !current.empty()) {
            args.push_back(current);
        }
        
        return args;
    }
    
    void printResult(const TaskResult& result) {
        if (result.success) {
            std::cout << Utils::GREEN << result.message << Utils::RESET << std::endl;
        } else {
            std::cout << Utils::RED << "Error: " << result.message << Utils::RESET << std::endl;
        }
    }

    // Helper method to validate task ID and return task pointer
    Task* validateAndFindTask(const std::string& idStr) {
        if (!Utils::isNumber(idStr)) {
            std::cout << Utils::RED << std::format(INVALID_TASK_ID_MSG, idStr) << Utils::RESET << std::endl;
            return nullptr;
        }
        
        int id = std::stoi(idStr);
        auto task = tasks.findTask(id);
        if (!task) {
            std::cout << Utils::RED << std::format(TASK_NOT_FOUND_MSG, id) << Utils::RESET << std::endl;
        }
        return task;
    }

public:
    TodoApp() : tasks("data/data.json") {}
    
    void run(int argc, char* argv[]) {
        if (argc < 2) {
            Utils::printHeader();
            std::cout << Utils::YELLOW << "Welcome to To-Do List Manager v2.0!" << Utils::RESET << std::endl;
            std::cout << "Use --help or -h for available commands." << std::endl;
            return;
        }
        
        std::vector<std::string> args = parseArguments(argc, argv);
        std::string command = Utils::toLowerCase(args[0]);
        
        try {
            if (command == "add") {
                handleAdd(args);
            } else if (command == "show") {
                handleShow(args);
            } else if (command == "update") {
                handleUpdate(args);
            } else if (command == "remove" || command == "rm") {
                handleRemove(args);
            } else if (command == "search") {
                handleSearch(args);
            } else if (command == "detail") {
                handleDetail(args);
            } else if (command == "overdue") {
                handleOverdue(args);
            } else if (command == "stats") {
                handleStats(args);
            } else if (command == "tag") {
                handleTag(args);
            } else if (command == "untag") {
                handleUntag(args);
            } else if (command == "due") {
                handleDue(args);
            } else if (command == "complete") {
                handleComplete(args);
            } else if (command == "--help" || command == "-h") {
                Utils::printHelp();
            } else if (command == "advanced-help" || command == "--advanced-help") {
                Utils::printAdvancedHelp();
            } else if (command == "--version" || command == "-v") {
                Utils::printVersion();
            } else {
                std::cout << Utils::RED << "Unknown command: " << command << Utils::RESET << std::endl;
                std::cout << "Use --help or -h for available commands." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        }
    }
private:
    void handleAdd(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << Utils::RED << "Usage: add <task_name> [status] [priority]" << Utils::RESET << std::endl;
            return;
        }
        
        std::string name = args[1];
        TaskStatus status = TaskStatus::TODO;
        TaskPriority priority = TaskPriority::LOW;
        
        // Parse status if provided
        if (args.size() > 2) {
            try {
                status = Utils::parseTaskStatus(args[2]);
            } catch (const std::exception& e) {
                std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
                return;
            }
        }
        
        // Parse priority if provided
        if (args.size() > 3) {
            try {
                priority = Utils::parseTaskPriority(args[3]);
            } catch (const std::exception& e) {
                std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
                return;
            }
        }
        
        auto result = tasks.addTask(name, status, priority);
        printResult(result);
    }
    
    void handleShow(const std::vector<std::string>& args) {
        if (args.size() == 1) {
            tasks.showAllTasks();
        } else if (args.size() == 2) {
            std::string filter = Utils::toLowerCase(args[1]);
            
            try {
                // Try to parse as status
                if (filter == "todo" || filter == "inprogress" || filter == "completed") {
                    TaskStatus status = Utils::parseTaskStatus(filter);
                    tasks.showFilteredTasks(status);
                    return;
                }
                
                // Try to parse as priority
                if (filter == "low" || filter == "medium" || filter == "high") {
                    TaskPriority priority = Utils::parseTaskPriority(filter);
                    tasks.showFilteredTasks(priority);
                    return;
                }
                
                std::cout << Utils::RED << "Invalid filter: " << filter << Utils::RESET << std::endl;
                std::cout << "Valid filters: todo, inprogress, completed, low, medium, high" << std::endl;
                
            } catch (const std::exception& e) {
                std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
            }
        } else {
            std::cout << Utils::RED << "Usage: show [filter]" << Utils::RESET << std::endl;
        }
    }
    
    void handleUpdate(const std::vector<std::string>& args) {
        if (args.size() != 5) {
            std::cout << Utils::RED << "Usage: update <task_ID> <name> <status> <priority>" << Utils::RESET << std::endl;
            return;
        }
        
        try {
            int id = std::stoi(args[1]);
            std::string name = args[2];
            TaskStatus status = Utils::parseTaskStatus(args[3]);
            TaskPriority priority = Utils::parseTaskPriority(args[4]);
            
            auto result = tasks.updateTask(id, name, status, priority);
            printResult(result);
            
        } catch (const std::exception& e) {
            std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        }
    }
    
    void handleRemove(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << USAGE_REMOVE_MSG << Utils::RESET << std::endl;
            return;
        }
        
        if (!Utils::isNumber(args[1])) {
            std::cout << Utils::RED << std::format(INVALID_TASK_ID_MSG, args[1]) << Utils::RESET << std::endl;
            return;
        }
        
        int id = std::stoi(args[1]);
        auto result = tasks.removeTask(id);
        printResult(result);
    }
    
    void handleSearch(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << "Usage: search <query>" << Utils::RESET << std::endl;
            return;
        }
        
        std::string searchTerm = args[1];
        auto results = tasks.searchTasks(searchTerm);
        
        if (results.empty()) {
            std::cout << Utils::YELLOW << "No tasks found matching: " << searchTerm << Utils::RESET << std::endl;
            return;
        }
        
        tasks.displayTaskList(results, std::format("Search results for: {}", searchTerm));
    }
    
    void handleDetail(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << USAGE_DETAIL_MSG << Utils::RESET << std::endl;
            return;
        }
        
        if (!Utils::isNumber(args[1])) {
            std::cout << Utils::RED << std::format(INVALID_TASK_ID_MSG, args[1]) << Utils::RESET << std::endl;
            return;
        }
        
        int id = std::stoi(args[1]);
        tasks.showTaskDetails(id);
    }
    
    void handleOverdue(const std::vector<std::string>& args) {
        if (args.size() != 1) {
            std::cout << Utils::RED << "Usage: overdue" << Utils::RESET << std::endl;
            return;
        }
        
        tasks.showOverdueTasks();
    }
    
    void handleStats(const std::vector<std::string>& args) {
        if (args.size() != 1) {
            std::cout << Utils::RED << "Usage: stats" << Utils::RESET << std::endl;
            return;
        }
        
        tasks.showStatistics();
    }
    
    void handleTag(const std::vector<std::string>& args) {
        if (args.size() != 3) {
            std::cout << Utils::RED << USAGE_TAG_MSG << Utils::RESET << std::endl;
            return;
        }
        
        auto task = validateAndFindTask(args[1]);
        if (!task) return;
        
        std::string tag = args[2];
        task->addTag(tag);
        tasks.save();
        std::cout << Utils::GREEN << TAG_ADDED_MSG << Utils::RESET << std::endl;
    }
    
    void handleUntag(const std::vector<std::string>& args) {
        if (args.size() != 3) {
            std::cout << Utils::RED << USAGE_UNTAG_MSG << Utils::RESET << std::endl;
            return;
        }
        
        auto task = validateAndFindTask(args[1]);
        if (!task) return;
        
        std::string tag = args[2];
        task->removeTag(tag);
        tasks.save();
        std::cout << Utils::GREEN << TAG_REMOVED_MSG << Utils::RESET << std::endl;
    }
    
    void handleDue(const std::vector<std::string>& args) {
        if (args.size() != 3) {
            std::cout << Utils::RED << USAGE_DUE_MSG << Utils::RESET << std::endl;
            return;
        }
        
        auto task = validateAndFindTask(args[1]);
        if (!task) return;
        
        auto dueDate = Utils::parseDate(args[2]);
        if (!dueDate) {
            std::cout << Utils::RED << INVALID_DATE_FORMAT_MSG << Utils::RESET << std::endl;
            return;
        }
        
        task->setDueDate(dueDate);
        tasks.save();
        std::cout << Utils::GREEN << DUE_DATE_SET_MSG << Utils::RESET << std::endl;
    }
    
    void handleComplete(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << "Usage: complete <task_ID>" << Utils::RESET << std::endl;
            return;
        }
        
        int id = std::stoi(args[1]);
        
        if (auto task = tasks.findTask(id)) {
            task->markCompleted();
            tasks.save();
            std::cout << Utils::GREEN << "Task marked as completed!" << Utils::RESET << std::endl;
        } else {
            std::cout << Utils::RED << "Task with ID " << id << " not found!" << Utils::RESET << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    try {
        TodoApp app;
        app.run(argc, argv);
    } catch (const std::exception& e) {
        std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        return 1;
    }
    
    return 0;
}
