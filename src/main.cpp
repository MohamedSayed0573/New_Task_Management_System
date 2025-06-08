#include "Tasks.hpp"
#include "utils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>

class TodoApp {
private:
    Tasks tasks;
    
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
    
public:
    TodoApp() : tasks("data/data.json") {}
    
    void run(int argc, char* argv[]) {
        if (argc < 2) {
            Utils::printHeader();
            std::cout << Utils::YELLOW << "Welcome to To-Do List Manager!" << Utils::RESET << std::endl;
            std::cout << "Use --help or -h for available commands." << std::endl;
            return;
        }
        
        std::vector<std::string> args = parseArguments(argc, argv);
        std::string command = args[0];
        
        // Convert command to lowercase for case-insensitive matching
        std::transform(command.begin(), command.end(), command.begin(), ::tolower);
        
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
        } else if (command == "--help" || command == "-h") {
            Utils::printHelp();
        } else if (command == "--version" || command == "-v") {
            Utils::printVersion();
        } else {
            std::cout << Utils::RED << "Unknown command: " << command << Utils::RESET << std::endl;
            std::cout << "Use --help or -h for available commands." << std::endl;
        }
    }
    
private:
    void handleAdd(const std::vector<std::string>& args) {
        if (args.size() < 2) {
            std::cout << Utils::RED << "Usage: add <task_name> [status] [priority]" << Utils::RESET << std::endl;
            return;
        }
        
        std::string name = args[1];
        int status = 1; // Default: To-Do
        int priority = 1; // Default: Low
        
        if (args.size() > 2 && Utils::isNumber(args[2])) {
            status = std::stoi(args[2]);
        }
        
        if (args.size() > 3 && Utils::isNumber(args[3])) {
            priority = std::stoi(args[3]);
        }
        
        tasks.addTask(name, status, priority);
    }
    
    void handleShow(const std::vector<std::string>& args) {
        if (args.size() == 1) {
            tasks.showAllTasks();
        } else if (args.size() == 2) {
            std::string filter = args[1];
            if (filter == "-t" || filter == "-i" || filter == "-c" ||
                filter == "-l" || filter == "-m" || filter == "-h") {
                tasks.showFilteredTasks(filter);
            } else {
                std::cout << Utils::RED << "Invalid filter: " << filter << Utils::RESET << std::endl;
                std::cout << "Valid filters: -t (To-Do), -i (In Progress), -c (Completed), -l (Low), -m (Medium), -h (High)" << std::endl;
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
        
        if (!Utils::isNumber(args[1])) {
            std::cout << Utils::RED << "Invalid task ID: " << args[1] << Utils::RESET << std::endl;
            return;
        }
        
        if (!Utils::isNumber(args[3])) {
            std::cout << Utils::RED << "Invalid status: " << args[3] << Utils::RESET << std::endl;
            return;
        }
        
        if (!Utils::isNumber(args[4])) {
            std::cout << Utils::RED << "Invalid priority: " << args[4] << Utils::RESET << std::endl;
            return;
        }
        
        int id = std::stoi(args[1]);
        std::string name = args[2];
        int status = std::stoi(args[3]);
        int priority = std::stoi(args[4]);
        
        tasks.updateTask(id, name, status, priority);
    }
    
    void handleRemove(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << "Usage: remove <task_ID>" << Utils::RESET << std::endl;
            return;
        }
        
        if (!Utils::isNumber(args[1])) {
            std::cout << Utils::RED << "Invalid task ID: " << args[1] << Utils::RESET << std::endl;
            return;
        }
        
        int id = std::stoi(args[1]);
        tasks.removeTask(id);
    }
    
    void handleSearch(const std::vector<std::string>& args) {
        if (args.size() != 2) {
            std::cout << Utils::RED << "Usage: search <task_name>" << Utils::RESET << std::endl;
            return;
        }
        
        std::string searchTerm = args[1];
        auto results = tasks.searchTasks(searchTerm);
        
        if (results.empty()) {
            std::cout << Utils::YELLOW << "No tasks found matching: " << searchTerm << Utils::RESET << std::endl;
            return;
        }
        
        Utils::printHeader();
        std::cout << Utils::BOLD << "Search results for: " << searchTerm << Utils::RESET << std::endl;
        Utils::printSeparator();
        
        std::cout << Utils::BOLD << std::left 
                  << std::setw(4) << "ID"
                  << std::setw(25) << "Name"
                  << std::setw(12) << "Status"
                  << std::setw(8) << "Priority" << Utils::RESET << std::endl;
        Utils::printSeparator();
        
        for (const auto& task : results) {
            std::cout << task->toString() << std::endl;
        }
        
        Utils::printSeparator();
        std::cout << Utils::CYAN << "Found " << results.size() << " task(s)" << Utils::RESET << std::endl;
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
