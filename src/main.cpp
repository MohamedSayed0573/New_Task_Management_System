#include "Tasks.hpp"
#include "utils.hpp"
#include <iostream>
#include <memory>
#include <format>
#include <vector>
#include <string>
#include <string_view>
#include <algorithm>

/**
 * Modern CLI application for todo task management
 * Custom command-line parser without external dependencies
 */
class CommandLineParser {
private:
    std::vector<std::string> args_;
    size_t current_arg_ = 0;

public:
    explicit CommandLineParser(int argc, char* argv[]) {
        args_.reserve(argc);
        for (int i = 0; i < argc; ++i) {
            args_.emplace_back(argv[i]);
        }
    }

    [[nodiscard]] bool hasMoreArgs() const noexcept {
        return current_arg_ < args_.size();
    }

    [[nodiscard]] std::string_view peekArg() const {
        if (current_arg_ >= args_.size()) {
            return "";
        }
        return args_[current_arg_];
    }

    [[nodiscard]] std::string_view nextArg() {
        if (current_arg_ >= args_.size()) {
            return "";
        }
        return args_[current_arg_++];
    }

    [[nodiscard]] std::string_view getCommand() {
        if (args_.size() < 2) {
            return "";
        }
        current_arg_ = 2; // Skip program name and get command
        return args_[1];
    }

    [[nodiscard]] bool isOption(std::string_view arg) const noexcept {
        return arg.starts_with("-");
    }

    [[nodiscard]] std::string_view getOptionValue(std::string_view option) {
        for (size_t i = current_arg_; i < args_.size(); ++i) {
            if (args_[i] == option && i + 1 < args_.size()) {
                return args_[i + 1];
            }
        }
        return "";
    }

    [[nodiscard]] bool hasOption(std::string_view option) const {
        return std::find(args_.begin() + 1, args_.end(), option) != args_.end();
    }

    [[nodiscard]] std::vector<std::string> getTagsFromOption(std::string_view option) {
        std::vector<std::string> tags;
        auto value = getOptionValue(option);
        if (!value.empty()) {
            tags = Utils::split(value, ',');
        }
        return tags;
    }

    void reset() noexcept {
        current_arg_ = 2; // Reset to after program name and command
    }
};

class TodoApplication {
private:
    std::unique_ptr<Tasks> tasks_;

    // Application configuration
    struct Config {
        std::string data_file = "data/data.json";
        bool verbose = false;
        bool quiet = false;
    } config_;

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

        std::cout << "  🗑️  remove <id>                   Delete a task (aliases: rm, delete)\n\n";

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

    void printVersion() const {
        std::cout << "To-Do List Manager v2.0.0 Enhanced Edition\n";
        std::cout << "Built with C++20/23 (No external dependencies)\n";
        std::cout << "Copyright (c) 2025 - Task Management System\n";
    }

    void parseGlobalOptions(CommandLineParser& parser) {
        if (parser.hasOption("--data-file")) {
            auto dataFile = parser.getOptionValue("--data-file");
            if (!dataFile.empty()) {
                config_.data_file = dataFile;
                tasks_ = std::make_unique<Tasks>(config_.data_file);
            }
        }

        config_.verbose = parser.hasOption("-v") || parser.hasOption("--verbose");
        config_.quiet = parser.hasOption("-q") || parser.hasOption("--quiet");
    }

    // Helper method to get option value with fallback
    std::string getOptionValueWithFallback(CommandLineParser& parser,
        std::string_view short_opt,
        std::string_view long_opt = "") {
        auto value = parser.getOptionValue(short_opt);
        if (value.empty() && !long_opt.empty()) {
            value = parser.getOptionValue(long_opt);
        }
        return std::string{ value };
    }

    // Helper method to parse and validate task ID
    std::optional<int> parseTaskId(CommandLineParser& parser, std::string_view command_name) {
        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task ID is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo " << command_name << " <id> ..." << std::endl;
            return std::nullopt;
        }

        auto id_str = parser.nextArg();
        if (!Utils::isNumber(id_str)) {
            std::cout << Utils::RED << "Error: Invalid task ID" << Utils::RESET << std::endl;
            return std::nullopt;
        }

        return std::stoi(std::string{ id_str });
    }

    // Command handlers
    void handleAddCommand(CommandLineParser& parser) {
        parser.reset();

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task name is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo add <name> [options]" << std::endl;
            return;
        }

        std::string name{ parser.nextArg() };

        // Parse options with simplified helper
        std::string status_str = getOptionValueWithFallback(parser, "-s", "--status");
        if (status_str.empty()) status_str = "todo";

        std::string priority_str = getOptionValueWithFallback(parser, "-p", "--priority");
        if (priority_str.empty()) priority_str = "low";

        std::string description = getOptionValueWithFallback(parser, "-d", "--description");
        std::string due_date_str = getOptionValueWithFallback(parser, "--due");

        std::vector<std::string> tags;
        std::string tags_str = getOptionValueWithFallback(parser, "-t", "--tags");
        if (!tags_str.empty()) {
            tags = Utils::split(tags_str, ',');
        }

        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Adding new task..." << Utils::RESET << std::endl;
            }

            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            std::optional<std::chrono::system_clock::time_point> due_date;
            if (!due_date_str.empty()) {
                due_date = Utils::parseDate(due_date_str);
                if (!due_date) {
                    throw std::invalid_argument("Invalid date format. Use YYYY-MM-DD");
                }
            }

            auto result = tasks_->addTask(name, description, status, priority, due_date, tags);

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

    void handleListCommand(CommandLineParser& parser) {
        parser.reset();
        std::string filter;

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

            if (filter.empty()) {
                tasks_->showAllTasks();
                return;
            }

            // Handle status filters
            if (filter == "todo" || filter == "inprogress" || filter == "completed") {
                TaskStatus status = Utils::parseTaskStatus(filter);
                tasks_->showFilteredTasks(status);
                return;
            }

            // Handle priority filters
            if (filter == "low" || filter == "medium" || filter == "high") {
                TaskPriority priority = Utils::parseTaskPriority(filter);
                tasks_->showFilteredTasks(priority);
                return;
            }

            // Handle special filters
            if (filter == "overdue") {
                tasks_->showOverdueTasks();
                return;
            }

            std::cout << Utils::YELLOW << "Unknown filter: " << filter << Utils::RESET << std::endl;
            std::cout << "Available filters: todo, inprogress, completed, low, medium, high, overdue" << std::endl;

        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Error filtering tasks: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleUpdateCommand(CommandLineParser& parser) {
        parser.reset();

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

            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            auto result = tasks_->updateTask(*id, name, status, priority);

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

    void handleRemoveCommand(CommandLineParser& parser) {
        parser.reset();

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

            auto results = tasks_->searchTasks(query);

            if (results.empty()) {
                std::cout << Utils::YELLOW << "No tasks found matching: \"" << query << "\"" << Utils::RESET << std::endl;
                return;
            }

            tasks_->displayTaskList(results, std::format("Search results for: \"{}\"", query));
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Search failed: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleDetailCommand(CommandLineParser& parser) {
        parser.reset();

        if (!parser.hasMoreArgs()) {
            std::cout << Utils::RED << "Error: Task ID is required" << Utils::RESET << std::endl;
            std::cout << "Usage: todo detail <id>" << std::endl;
            return;
        }

        auto id_str = parser.nextArg();
        if (!Utils::isNumber(id_str)) {
            std::cout << Utils::RED << "Error: Invalid task ID" << Utils::RESET << std::endl;
            return;
        }
        int id = std::stoi(std::string{ id_str });

        try {
            tasks_->showTaskDetails(id);
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show task details: " << e.what() << Utils::RESET << std::endl;
        }
    }

    // Helper method to execute task operations
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

    void handleCompleteCommand(CommandLineParser& parser) {
        parser.reset();

        auto id = parseTaskId(parser, "complete");
        if (!id) return;

        executeTaskOperation(*id, "Marking as completed", [](Task* task) {
            task->markCompleted();
            });
    }

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

            auto due_date = Utils::parseDate(date_str);
            if (!due_date) {
                std::cout << Utils::RED << "✗ Invalid date format. Use YYYY-MM-DD" << Utils::RESET << std::endl;
                return;
            }

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

    void handleStatsCommand() {
        try {
            tasks_->showStatistics();
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show statistics: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleOverdueCommand() {
        try {
            tasks_->showOverdueTasks();
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show overdue tasks: " << e.what() << Utils::RESET << std::endl;
        }
    }

public:
    TodoApplication() {
        tasks_ = std::make_unique<Tasks>(config_.data_file);
    }

    int run(int argc, char* argv[]) {
        CommandLineParser parser(argc, argv);

        // Check for version or help
        if (parser.hasOption("--version")) {
            printVersion();
            return 0;
        }

        if (parser.hasOption("-h") || parser.hasOption("--help") || argc < 2) {
            printUsage();
            return 0;
        }

        // Parse global options
        parseGlobalOptions(parser);

        // Get command
        auto command = parser.getCommand();
        if (command.empty()) {
            std::cout << Utils::RED << "Error: No command specified" << Utils::RESET << std::endl;
            printUsage();
            return 1;
        }

        try {
            // Route to appropriate command handler
            if (command == "add") {
                handleAddCommand(parser);
            }
            else if (command == "list" || command == "ls") {
                handleListCommand(parser);
            }
            else if (command == "update") {
                handleUpdateCommand(parser);
            }
            else if (command == "remove" || command == "rm" || command == "delete") {
                handleRemoveCommand(parser);
            }
            else if (command == "search" || command == "find") {
                handleSearchCommand(parser);
            }
            else if (command == "detail" || command == "show" || command == "info") {
                handleDetailCommand(parser);
            }
            else if (command == "complete" || command == "done") {
                handleCompleteCommand(parser);
            }
            else if (command == "tag") {
                handleTagCommand(parser);
            }
            else if (command == "untag") {
                handleUntagCommand(parser);
            }
            else if (command == "due" || command == "deadline") {
                handleDueDateCommand(parser);
            }
            else if (command == "stats" || command == "statistics") {
                handleStatsCommand();
            }
            else if (command == "overdue") {
                handleOverdueCommand();
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
