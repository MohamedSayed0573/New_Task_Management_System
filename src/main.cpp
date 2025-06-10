/**
 * @file main.cpp
 * @brief Main application entry point for the Todo List Manager
 *
 * This file contains the main application logic including:
 * - Custom command-line parser for handling user input
 * - TodoApplication class with comprehensive command handlers
 * - Modern C++ features and error handling
 * - Extensive help system and usage information
 */

#include "Tasks.hpp"
#include "utils.hpp"
#include <iostream>
#include <memory>
#include <format>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>
#include <functional> // Added for std::function
#include <unordered_map> // Added for std::unordered_map
#include <unordered_set> // Added for std::unordered_set
#include <optional>      // Added for std::optional

 /**
  * @class CommandLineParser
  * @brief Custom command-line argument parser without external dependencies
  *
  * Provides comprehensive parsing of command-line arguments including:
  * - Command extraction and validation
  * - Option parsing with short and long forms
  * - Argument validation and type checking
  * - Support for complex option patterns
  */
class CommandLineParser {
private:
    std::vector<std::string> args_;             ///< Stored command-line arguments
    std::vector<std::string> positional_args_; ///< Non-option arguments after command
    std::unordered_map<std::string, std::string> options_; ///< Parsed options with values
    std::unordered_set<std::string> flags_;     ///< Boolean flags (options without values)
    size_t current_arg_ = 0;                   ///< Current position in argument list
    bool parsed_ = false;                      ///< Whether arguments have been parsed

public:
    /**
     * @brief Construct parser from command-line arguments
     * @param argc Number of arguments
     * @param argv Array of argument strings
     */
    explicit CommandLineParser(int argc, char* argv[]) {
        args_.reserve(argc);
        // Store all arguments for processing
        for (int i = 0; i < argc; ++i) {
            args_.emplace_back(argv[i]);
        }
        parseArguments();
    }

private:
    /**
     * @brief Parse all arguments into options and positional arguments
     */
    void parseArguments() {
        if (parsed_ || args_.size() < 2) return;
        
        // Skip program name and command
        for (size_t i = 2; i < args_.size(); ++i) {
            const auto& arg = args_[i];
            
            if (isOption(arg)) {
                // Handle option with potential value
                if (i + 1 < args_.size() && !isOption(args_[i + 1])) {
                    // Option with value
                    options_[arg] = args_[i + 1];
                    ++i; // Skip the value
                } else {
                    // Boolean flag
                    flags_.insert(arg);
                }
            } else {
                // Positional argument
                positional_args_.push_back(arg);
            }
        }
        parsed_ = true;
    }

public:

    // ========================
    // Argument Navigation
    // ========================

    /**
     * @brief Check if more positional arguments are available
     * @return true if more positional arguments exist
     */
    [[nodiscard]] bool hasMoreArgs() const noexcept {
        return current_arg_ < positional_args_.size();
    }

    /**
     * @brief Peek at next positional argument without consuming it
     * @return Next argument or empty string if none available
     */
    [[nodiscard]] std::string_view peekArg() const {
        if (current_arg_ >= positional_args_.size()) {
            return "";
        }
        return positional_args_[current_arg_];
    }

    /**
     * @brief Get next positional argument and advance position
     * @return Next argument or empty string if none available
     */
    [[nodiscard]] std::string_view nextArg() {
        if (current_arg_ >= positional_args_.size()) {
            return "";
        }
        return positional_args_[current_arg_++];
    }

    /**
     * @brief Extract command from arguments (assumes position 1)
     * @return Command string or empty if not available
     */
    [[nodiscard]] std::string_view getCommand() {
        if (args_.size() < 2) {
            return "";
        }
        current_arg_ = 0; // Reset to start of positional args
        return args_[1];
    }

    // ===================
    // Option Processing
    // ===================

    /**
     * @brief Check if argument is an option (starts with -)
     * @param arg Argument to check
     * @return true if argument is an option
     */
    [[nodiscard]] bool isOption(std::string_view arg) const noexcept {
        return arg.starts_with("-");
    }

    /**
     * @brief Get value for a specific option
     * @param option Option name to search for
     * @return Option value or empty string if not found
     */
    [[nodiscard]] std::string_view getOptionValue(std::string_view option) const {
        auto it = options_.find(std::string(option));
        if (it != options_.end()) {
            return it->second;
        }
        return "";
    }

    /**
     * @brief Check if a specific option exists (with or without value)
     * @param option Option name to check
     * @return true if option is present
     */
    [[nodiscard]] bool hasOption(std::string_view option) const {
        return options_.contains(std::string(option)) || flags_.contains(std::string(option));
    }

    /**
     * @brief Parse comma-separated tags from option value
     * @param option Option name containing tag list
     * @return Vector of individual tags
     */
    [[nodiscard]] std::vector<std::string> getTagsFromOption(std::string_view option) {
        std::vector<std::string> tags;
        auto value = getOptionValue(option);
        if (!value.empty()) {
            tags = Utils::split(value, ',');
        }
        return tags;
    }

    /**
     * @brief Reset parser position to start of positional arguments
     */
    void reset() noexcept {
        current_arg_ = 0; // Reset to start of positional args
    }
};

/**
 * @class TodoApplication
 * @brief Main application class handling all todo operations
 *
 * Provides a comprehensive interface for todo management including:
 * - All CRUD operations for tasks
 * - Advanced search and filtering
 * - Statistics and reporting
 * - Multiple output formats
 * - Comprehensive error handling
 */
class TodoApplication {
private:
    std::unique_ptr<Tasks> tasks_;    ///< Main task container
    std::unordered_map<std::string, std::function<void(CommandLineParser&)>> command_handlers_; ///< Command dispatcher

    /**
     * @struct Config
     * @brief Application configuration settings
     */
    struct Config {
        std::string data_file = "data/data.json";  ///< Path to data file
        bool verbose = false;                      ///< Enable verbose output
        bool quiet = false;                        ///< Suppress non-essential output
    } config_;

    // ==================
    // Help and Usage
    // ==================

    /**
     * @brief Display comprehensive usage information
     */
    void printUsage() const {
        std::cout << Utils::BOLD << "📋 To-Do List Manager v2.0 - Enhanced Edition" << Utils::RESET << "\n\n";

        std::cout << Utils::CYAN << "USAGE:" << Utils::RESET << "\n";
        std::cout << "  todo <command> [options] [arguments]\n\n";

        std::cout << Utils::CYAN << "GLOBAL OPTIONS:" << Utils::RESET << "\n";
        std::cout << "  --data-file <path>    Specify custom data file path\n";
        std::cout << "  -v, --verbose         Enable detailed output\n";
        std::cout << "  -q, --quiet          Suppress non-essential output\n";
        std::cout << "  --version            Show version information\n";
        std::cout << "  -h, --help           Show this help message\n\n";

        std::cout << Utils::CYAN << "COMMANDS:" << Utils::RESET << "\n";
        std::cout << "  ✅ add <name>                     Create a new task\n";
        std::cout << "     Options: -s|--status <status>, -p|--priority <priority>\n";
        std::cout << "              -d|--description <text>, --due <date>\n";
        std::cout << "              -t|--tags <tag1,tag2,...>\n\n";

        std::cout << "  📋 list [filter]                  Display tasks (aliases: ls)\n";
        std::cout << "     Filters: todo, inprogress, completed, low, medium, high, overdue\n\n";

        std::cout << "  🔄 update <id> <name> <status> <priority>  Modify existing task\n\n";

        std::cout << "  🗑️  remove <id>                   Delete a task (aliases: rm, delete)\n";
        std::cout << "     Options: --all (remove all tasks with confirmation)\n\n";

        std::cout << "  🔍 search <query>                 Find tasks (aliases: find)\n\n";

        std::cout << "  📖 detail <id>                    Show task details (aliases: show, info)\n\n";

        std::cout << "  ✅ complete <id>                  Mark task as completed (aliases: done)\n\n";

        std::cout << "  🏷️  tag <id> <tag>                Add tag to task\n\n";

        std::cout << "  🏷️❌ untag <id> <tag>             Remove tag from task\n\n";

        std::cout << "  📅 due <id> <date>                Set due date (aliases: deadline)\n\n";

        std::cout << "  📊 stats                          Show statistics (aliases: statistics)\n\n";

        std::cout << "  ⚠️  overdue                       Show overdue tasks\n\n";        std::cout << Utils::CYAN << "EXAMPLES:" << Utils::RESET << "\n";
        std::cout << "  todo add \"Buy groceries\" --priority high --due 2025-12-31\n";
        std::cout << "  todo add \"Write report\" -p medium -d \"Quarterly analysis\" -t work,urgent\n";
        std::cout << "  todo list completed\n";
        std::cout << "  todo search \"grocery\"\n";
        std::cout << "  todo complete 1\n";
        std::cout << "  todo tag 2 urgent\n\n";

        std::cout << Utils::CYAN << "VALID VALUES:" << Utils::RESET << "\n";
        std::cout << "  Status: todo, inprogress, completed\n";
        std::cout << "  Priority: low, medium, high\n";
        std::cout << "  Date format: YYYY-MM-DD (e.g., 2025-12-31)\n";
    }

    /**
     * @brief Display version information
     */
    void printVersion() const {
        std::cout << "To-Do List Manager v2.0.0 Enhanced Edition\n";
        std::cout << "Built with C++20/23 (No external dependencies)\n";
        std::cout << "Copyright (c) 2025 - Task Management System\n";
    }

    /**
     * @brief Parse and apply global command-line options
     * @param parser Command line parser instance
     */
    void parseGlobalOptions(CommandLineParser& parser) {
        // Handle custom data file option
        if (parser.hasOption("--data-file")) {
            auto dataFile = parser.getOptionValue("--data-file");
            if (!dataFile.empty()) {
                config_.data_file = dataFile;
                tasks_ = std::make_unique<Tasks>(config_.data_file);
            }
        }

        // Set verbosity flags
        config_.verbose = parser.hasOption("-v") || parser.hasOption("--verbose");
        config_.quiet = parser.hasOption("-q") || parser.hasOption("--quiet");
    }

    // =======================
    // Helper Utility Methods
    // =======================

    /**
     * @brief Get option value with fallback to alternative option name
     * @param parser Command line parser
     * @param short_opt Short option name (e.g., "-p")
     * @param long_opt Long option name (e.g., "--priority")
     * @return Option value or empty string if not found
     */
    std::string getOptionValueWithFallback(CommandLineParser& parser,
        std::string_view short_opt,
        std::string_view long_opt = "") {
        auto value = parser.getOptionValue(short_opt);
        if (value.empty() && !long_opt.empty()) {
            value = parser.getOptionValue(long_opt);
        }
        return std::string{ value };
    }

    /**
     * @brief Parse and validate task ID from arguments
     * @param parser Command line parser
     * @param command_name Name of current command (for error messages)
     * @return Optional task ID if valid, nullopt otherwise
     */
    std::optional<int> parseTaskId(CommandLineParser& parser, std::string_view command_name) {
        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task ID is required for " << command_name << Utils::RESET << std::endl;
            return std::nullopt;
        }

        auto id_str = parser.nextArg();
        if (!Utils::isNumber(id_str)) {
            std::cout << Utils::RED << "Error: Invalid task ID for " << command_name << Utils::RESET << std::endl;
            return std::nullopt;
        }

        return std::stoi(std::string{ id_str });
    }

    // =====================================
    // Command Handler Methods
    // =====================================

    /**
     * @brief Handle 'add' command - create new task
     * @param parser Command line parser
     */
    void handleAddCommand(CommandLineParser& parser) {
        parser.reset();

        // Validate required task name (first positional argument)
        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task name is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo add <name> [options]" << std::endl;
            std::cout << "Options:" << std::endl;
            std::cout << "  -s, --status <status>     Task status (todo|inprogress|completed)" << std::endl;
            std::cout << "  -p, --priority <priority> Task priority (low|medium|high)" << std::endl;
            std::cout << "  -d, --description <desc>  Task description" << std::endl;
            std::cout << "  --due <date>              Due date (YYYY-MM-DD)" << std::endl;
            std::cout << "  -t, --tags <tags>         Comma-separated tags" << std::endl;
            std::cout << std::endl;
            std::cout << "Examples:" << std::endl;
            std::cout << "  todo add \"Learn C++\"" << std::endl;
            std::cout << "  todo add --priority high \"Important task\" --due 2024-12-31" << std::endl;
            std::cout << "  todo add \"My task\" -s inprogress -p medium -t work,coding" << std::endl;
            return;
        }

        std::string name{ parser.nextArg() };

        // Parse optional parameters with defaults (now order-independent)
        std::string status_str = getOptionValueWithFallback(parser, "-s", "--status");
        if (status_str.empty()) status_str = "todo";

        std::string priority_str = getOptionValueWithFallback(parser, "-p", "--priority");
        if (priority_str.empty()) priority_str = "low";

        std::string description = getOptionValueWithFallback(parser, "-d", "--description");
        std::string due_date_str = getOptionValueWithFallback(parser, "--due");

        // Parse comma-separated tags
        std::vector<std::string> tags;
        std::string tags_str = getOptionValueWithFallback(parser, "-t", "--tags");
        if (!tags_str.empty()) {
            tags = Utils::split(tags_str, ',');
        }

        try {
            // Parse and validate status/priority
            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            // Parse optional due date
            std::optional<std::chrono::system_clock::time_point> due_date;
            if (!due_date_str.empty()) {
                due_date = Utils::parseDate(due_date_str);
                if (!due_date) {
                    throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD");
                }
            }

            // Execute task creation
            auto result = tasks_->addTask(name, description, status, priority, due_date, tags);

            // Display result with appropriate formatting
            if (result.success) {
                std::cout << Utils::GREEN << "✓ " << result.message << Utils::RESET << std::endl;
                if (config_.verbose) {
                    std::cout << Utils::BLUE << "Task details: " << name
                        << " [" << status_str << ", " << priority_str << "]"
                        << Utils::RESET << std::endl;
                }
            }
            else {
                std::cout << Utils::RED << "✗ Error: " << result.message << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to add task: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'list' command - display tasks with optional filtering
     * @param parser Command line parser
     */
    void handleListCommand(CommandLineParser& parser) {
        parser.reset();
        std::string filter;

        // Extract optional filter parameter
        if (parser.hasMoreArgs()) {
            auto next = parser.peekArg();
            if (!parser.isOption(next)) {
                filter = parser.nextArg();
            }
        }

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Listing tasks..." << Utils::RESET << std::endl;
            }

            // Handle different filter types
            if (filter.empty()) {
                tasks_->showAllTasks();
                return;
            }

            // Status-based filters
            if (filter == "todo" || filter == "inprogress" || filter == "completed") {
                TaskStatus status = Utils::parseTaskStatus(filter);
                tasks_->showFilteredTasks(status);
                return;
            }

            // Priority-based filters
            if (filter == "low" || filter == "medium" || filter == "high") {
                TaskPriority priority = Utils::parseTaskPriority(filter);
                tasks_->showFilteredTasks(priority);
                return;
            }

            // Special filters
            if (filter == "overdue") {
                tasks_->showOverdueTasks();
                return;
            }

            // Unknown filter
            std::cout << Utils::YELLOW << "Unknown filter: " << filter << Utils::RESET << std::endl;
            std::cout << "Available filters: todo, inprogress, completed, low, medium, high, overdue" << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Error filtering tasks: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'update' command - modify existing task
     * @param parser Command line parser
     */
    void handleUpdateCommand(CommandLineParser& parser) {
        parser.reset();

        // Parse required parameters with validation
        auto id = parseTaskId(parser, "update");
        if (!id) return;

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task name is required" << Utils::RESET << std::endl;
            return;
        }
        std::string name{ parser.nextArg() };

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Status is required" << Utils::RESET << std::endl;
            return;
        }
        std::string status_str{ parser.nextArg() };

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Priority is required" << Utils::RESET << std::endl;
            return;
        }
        std::string priority_str{ parser.nextArg() };

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Updating task " << *id << "..." << Utils::RESET << std::endl;
            }

            // Parse and validate parameters
            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            // Execute update operation
            auto result = tasks_->updateTask(*id, name, status, priority);

            // Display result
            if (result.success) {
                std::cout << Utils::GREEN << "✓ " << result.message << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Error: " << result.message << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to update task: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'remove' command - delete task(s)
     * @param parser Command line parser
     */
    void handleRemoveCommand(CommandLineParser& parser) {
        parser.reset();

        // Handle bulk removal option
        if (parser.hasOption("--all")) {
            try {
                if (!config_.quiet) {
                    std::cout << Utils::CYAN << "Preparing to remove all tasks..." << Utils::RESET << std::endl;
                }

                // Validate there are tasks to remove
                size_t taskCount = tasks_->size();
                if (taskCount == 0) {
                    std::cout << Utils::YELLOW << "No tasks to remove!" << Utils::RESET << std::endl;
                    return;
                }

                // Confirm destructive operation
                std::cout << Utils::YELLOW << "You are about to remove " << taskCount << " task(s)!" << Utils::RESET << std::endl;

                if (!Utils::confirmAction("Are you sure you want to remove ALL tasks? This action cannot be undone.")) {
                    std::cout << Utils::CYAN << "Operation cancelled." << Utils::RESET << std::endl;
                    return;
                }

                // Execute bulk removal
                auto result = tasks_->removeAllTasks();

                if (result.success) {
                    std::cout << Utils::GREEN << "✓ " << result.message << Utils::RESET << std::endl;
                }
                else {
                    std::cout << Utils::RED << "✗ Error: " << result.message << Utils::RESET << std::endl;
                }
            }
            catch (const std::exception& e) {
                std::cout << Utils::RED << "✗ Failed to remove all tasks: " << e.what() << Utils::RESET << std::endl;
            }
            return;
        }

        // Handle single task removal
        auto id = parseTaskId(parser, "remove");
        if (!id) return;

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Removing task " << *id << "..." << Utils::RESET << std::endl;
            }

            auto result = tasks_->removeTask(*id);

            if (result.success) {
                std::cout << Utils::GREEN << "✓ " << result.message << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Error: " << result.message << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to remove task: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'search' command - find tasks by query
     * @param parser Command line parser
     */
    void handleSearchCommand(CommandLineParser& parser) {
        parser.reset();

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Search query is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo search <query>" << std::endl;
            return;
        }

        std::string query{ parser.nextArg() };

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Searching for: \"" << query << "\"..." << Utils::RESET << std::endl;
            }

            // Execute search operation
            auto results = tasks_->searchTasks(query);

            if (results.empty()) {
                std::cout << Utils::YELLOW << "No tasks found matching: \"" << query << "\"" << Utils::RESET << std::endl;
                return;
            }

            // Display search results
            tasks_->displayTaskList(results, std::format("Search results for: \"{}\"", query));
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Search failed: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'detail' command - show detailed task information
     * @param parser Command line parser
     */
    void handleDetailCommand(CommandLineParser& parser) {
        parser.reset();

        auto id_opt = parseTaskId(parser, "detail");
        if (!id_opt) {
            // parseTaskId already prints error messages for missing or invalid ID.
            return;
        }

        int id = *id_opt;

        try {
            tasks_->showTaskDetails(id);
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show task details: " << e.what() << Utils::RESET << std::endl;
        }
    }

    // =================================
    // Generic Task Operation Helper
    // =================================

    /**
     * @brief Execute a task operation with error handling
     * @tparam Operation Callable type for the operation
     * @param id Task ID to operate on
     * @param operation_name Description of operation for user feedback
     * @param op Operation function to execute
     */
    template<typename Operation>
    void executeTaskOperation(int id, std::string_view operation_name, Operation&& op) {
        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << operation_name << " task " << id << "..." << Utils::RESET << std::endl;
            }

            if (auto task = tasks_->findTask(id)) {
                op(task);
                tasks_->save();
                std::cout << Utils::GREEN << "✓ Operation completed successfully!" << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Task with ID " << id << " not found!" << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to " << operation_name << " task: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'complete' command - mark task as completed
     * @param parser Command line parser
     */
    void handleCompleteCommand(CommandLineParser& parser) {
        parser.reset();

        auto id = parseTaskId(parser, "complete");
        if (!id) return;

        executeTaskOperation(*id, "Marking as completed", [](Task* task) {
            task->markCompleted();
            });
    }

    /**
     * @brief Handle 'tag' command - add tag to task
     * @param parser Command line parser
     */
    void handleTagCommand(CommandLineParser& parser) {
        parser.reset();

        auto id = parseTaskId(parser, "tag");
        if (!id) return;

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Tag is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo tag <id> <tag>" << std::endl;
            return;
        }
        std::string tag{ parser.nextArg() };

        executeTaskOperation(*id, std::format("Adding tag \"{}\" to", tag), [&tag](Task* task) {
            task->addTag(tag);
            });
    }

    /**
     * @brief Handle 'untag' command - remove tag from task
     * @param parser Command line parser
     */
    void handleUntagCommand(CommandLineParser& parser) {
        parser.reset();

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task ID is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo untag <id> <tag>" << std::endl;
            return;
        }

        auto id_str = parser.nextArg();
        if (!Utils::isNumber(id_str)) {
            std::cout << Utils::RED << "Error: Invalid task ID" << Utils::RESET << std::endl;
            return;
        }
        int id = std::stoi(std::string{ id_str });

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Tag is required" << Utils::RESET << std::endl;
            return;
        }
        std::string tag{ parser.nextArg() };

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Removing tag \"" << tag << "\" from task " << id << "..." << Utils::RESET << std::endl;
            }

            if (auto task = tasks_->findTask(id)) {
                task->removeTag(tag);
                tasks_->save();
                std::cout << Utils::GREEN << "✓ Tag removed successfully!" << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Task with ID " << id << " not found!" << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to remove tag: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'due' command - set task due date
     * @param parser Command line parser
     */
    void handleDueDateCommand(CommandLineParser& parser) {
        parser.reset();

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task ID is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo due <id> <date>" << std::endl;
            return;
        }

        auto id_str = parser.nextArg();
        if (!Utils::isNumber(id_str)) {
            std::cout << Utils::RED << "Error: Invalid task ID" << Utils::RESET << std::endl;
            return;
        }
        int id = std::stoi(std::string{ id_str });

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Date is required" << Utils::RESET << std::endl;
            return;
        }
        std::string date_str{ parser.nextArg() };

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Setting due date for task " << id << "..." << Utils::RESET << std::endl;
            }

            // Parse and validate date
            auto due_date = Utils::parseDate(date_str);
            if (!due_date) {
                std::cout << Utils::RED << "✗ Invalid date format. Use YYYY-MM-DD" << Utils::RESET << std::endl;
                return;
            }

            // Execute due date setting
            if (auto task = tasks_->findTask(id)) {
                task->setDueDate(due_date);
                tasks_->save();
                std::cout << Utils::GREEN << "✓ Due date set successfully!" << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Task with ID " << id << " not found!" << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to set due date: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'stats' command - show task statistics
     */
    void handleStatsCommand() {
        try {
            tasks_->showStatistics();
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show statistics: " << e.what() << Utils::RESET << std::endl;
        }
    }

    /**
     * @brief Handle 'overdue' command - show overdue tasks
     */
    void handleOverdueCommand() {
        try {
            tasks_->showOverdueTasks();
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show overdue tasks: " << e.what() << Utils::RESET << std::endl;
        }
    }

public:
    /**
     * @brief Construct TodoApplication with default configuration
     */
    TodoApplication() {
        tasks_ = std::make_unique<Tasks>(config_.data_file);

        // Initialize command handlers
        command_handlers_["add"] = [this](CommandLineParser& p) { this->handleAddCommand(p); };
        command_handlers_["list"] = [this](CommandLineParser& p) { this->handleListCommand(p); };
        command_handlers_["ls"] = [this](CommandLineParser& p) { this->handleListCommand(p); };
        command_handlers_["update"] = [this](CommandLineParser& p) { this->handleUpdateCommand(p); };
        command_handlers_["remove"] = [this](CommandLineParser& p) { this->handleRemoveCommand(p); };
        command_handlers_["rm"] = [this](CommandLineParser& p) { this->handleRemoveCommand(p); };
        command_handlers_["delete"] = [this](CommandLineParser& p) { this->handleRemoveCommand(p); };
        command_handlers_["search"] = [this](CommandLineParser& p) { this->handleSearchCommand(p); };
        command_handlers_["find"] = [this](CommandLineParser& p) { this->handleSearchCommand(p); };
        command_handlers_["detail"] = [this](CommandLineParser& p) { this->handleDetailCommand(p); };
        command_handlers_["show"] = [this](CommandLineParser& p) { this->handleDetailCommand(p); };
        command_handlers_["info"] = [this](CommandLineParser& p) { this->handleDetailCommand(p); };
        command_handlers_["complete"] = [this](CommandLineParser& p) { this->handleCompleteCommand(p); };
        command_handlers_["done"] = [this](CommandLineParser& p) { this->handleCompleteCommand(p); };
        command_handlers_["tag"] = [this](CommandLineParser& p) { this->handleTagCommand(p); };
        command_handlers_["untag"] = [this](CommandLineParser& p) { this->handleUntagCommand(p); };
        command_handlers_["due"] = [this](CommandLineParser& p) { this->handleDueDateCommand(p); };
        command_handlers_["deadline"] = [this](CommandLineParser& p) { this->handleDueDateCommand(p); };
        command_handlers_["stats"] = [this](CommandLineParser&) { this->handleStatsCommand(); }; // Note: handleStatsCommand takes no parser
        command_handlers_["statistics"] = [this](CommandLineParser&) { this->handleStatsCommand(); };
        command_handlers_["overdue"] = [this](CommandLineParser&) { this->handleOverdueCommand(); }; // Note: handleOverdueCommand takes no parser
    }

    /**
     * @brief Main application entry point
     * @param argc Number of command-line arguments
     * @param argv Array of command-line arguments
     * @return Exit code (0 for success, non-zero for error)
     */
    int run(int argc, char* argv[]) {
        CommandLineParser parser(argc, argv);

        // Handle special options first
        if (parser.hasOption("--version")) {
            printVersion();
            return 0;
        }

        if (parser.hasOption("-h") || parser.hasOption("--help") || argc < 2) {
            printUsage();
            return 0;
        }

        // Parse global configuration options
        parseGlobalOptions(parser);

        // Extract and validate command
        auto command = parser.getCommand();
        if (command.empty()) {
            std::cout << Utils::RED << "Error: No command specified" << Utils::RESET << std::endl;
            printUsage();
            return 1;
        }

        try {
            // Route command to appropriate handler
            auto command_str = std::string{ command };
            auto it = command_handlers_.find(command_str);
            if (it != command_handlers_.end()) {
                it->second(parser); // Call the handler
            }
            else {
                std::cout << Utils::RED << "Error: Unknown command '" << command << "'" << Utils::RESET << std::endl;
                std::cout << "Use 'todo --help' for available commands" << std::endl;
                return 1;
            }

            return 0;
        }
        catch (const std::exception& e) {
            std::cerr << Utils::RED << "Unexpected error: " << e.what() << Utils::RESET << std::endl;
            return 1;
        }
    }
};

/**
 * @brief Application entry point with comprehensive error handling
 * @param argc Number of command-line arguments
 * @param argv Array of command-line arguments
 * @return Exit code
 */
int main(int argc, char* argv[]) {
    try {
        TodoApplication app;
        return app.run(argc, argv);
    }
    catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "Fatal unknown error occurred" << std::endl;
        return 1;
    }
}
