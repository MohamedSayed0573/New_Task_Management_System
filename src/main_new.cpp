#include "Tasks.hpp"
#include "utils.hpp"
#include "CLI11.hpp"
#include <iostream>
#include <memory>
#include <format>

/**
 * Modern CLI application for todo task management
 * Rewritten from scratch using CLI11 best practices
 */
class TodoApplication {
private:
    std::unique_ptr<Tasks> tasks_;

    // Application configuration
    struct Config {
        std::string data_file = "data/data.json";
        bool verbose = false;
        bool quiet = false;
    } config_;

    // Command handlers
    void handleVersionCommand() {
        std::cout << "To-Do List Manager v2.0.0 Enhanced Edition\n";
        std::cout << "Built with C++20/23 and CLI11\n";
        std::cout << "Copyright (c) 2024 - Task Management System\n";
    }

    void handleAddCommand(const std::string& name,
        const std::string& status_str = "todo",
        const std::string& priority_str = "low",
        const std::string& description = "",
        const std::string& due_date_str = "",
        const std::vector<std::string>& tags = {}) {
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

    void handleListCommand(const std::string& filter = "") {
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

    void handleUpdateCommand(int id,
        const std::string& name,
        const std::string& status_str,
        const std::string& priority_str) {
        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Updating task " << id << "..." << Utils::RESET << std::endl;
            }

            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            auto result = tasks_->updateTask(id, name, status, priority);

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

    void handleRemoveCommand(int id) {
        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Removing task " << id << "..." << Utils::RESET << std::endl;
            }

            auto result = tasks_->removeTask(id);

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

    void handleSearchCommand(const std::string& query) {
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

    void handleDetailCommand(int id) {
        try {
            tasks_->showTaskDetails(id);
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to show task details: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleCompleteCommand(int id) {
        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Marking task " << id << " as completed..." << Utils::RESET << std::endl;
            }

            if (auto task = tasks_->findTask(id)) {
                task->markCompleted();
                tasks_->save();
                std::cout << Utils::GREEN << "✓ Task marked as completed!" << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Task with ID " << id << " not found!" << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to complete task: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleTagCommand(int id, const std::string& tag) {
        try {
            if (!config_.quiet) {
                std::cout << Utils::CYAN << "Adding tag \"" << tag << "\" to task " << id << "..." << Utils::RESET << std::endl;
            }

            if (auto task = tasks_->findTask(id)) {
                task->addTag(tag);
                tasks_->save();
                std::cout << Utils::GREEN << "✓ Tag added successfully!" << Utils::RESET << std::endl;
            }
            else {
                std::cout << Utils::RED << "✗ Task with ID " << id << " not found!" << Utils::RESET << std::endl;
            }
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "✗ Failed to add tag: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleUntagCommand(int id, const std::string& tag) {
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

    void handleDueDateCommand(int id, const std::string& date_str) {
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
        // Create the main CLI app
        CLI::App app{ "To-Do List Manager v2.0 - A powerful, modern task management system" };

        // Set up global options
        app.add_option("--data-file", config_.data_file, "Path to the data file")
            ->check(CLI::ExistingPath | CLI::NonexistentPath);

        app.add_flag("-v,--verbose", config_.verbose, "Enable verbose output");
        app.add_flag("-q,--quiet", config_.quiet, "Suppress non-essential output");

        // Set up help and version
        app.set_version_flag("--version", [this]() { handleVersionCommand(); }, "Show version information");

        // Add footer with examples
        app.footer("Examples:\n"
            "  todo add \"Buy groceries\" --priority high --due 2024-12-31\n"
            "  todo list --filter completed\n"
            "  todo search \"grocery\"\n"
            "  todo complete 1\n\n"
            "For more help on a specific command, use: todo <command> --help");

        // Configure help
        app.set_help_all_flag("--help-all", "Show help for all commands");

        // Require at least one subcommand or show help
        app.require_subcommand(1);

        // ADD command
        auto* add_cmd = app.add_subcommand("add", "Add a new task");
        {
            std::string name, status = "todo", priority = "low", description, due_date;
            std::vector<std::string> tags;

            add_cmd->add_option("name", name, "Task name")->required();
            add_cmd->add_option("-s,--status", status, "Task status")
                ->check(CLI::IsMember({ "todo", "inprogress", "completed" }));
            add_cmd->add_option("-p,--priority", priority, "Task priority")
                ->check(CLI::IsMember({ "low", "medium", "high" }));
            add_cmd->add_option("-d,--description", description, "Task description");
            add_cmd->add_option("--due", due_date, "Due date (YYYY-MM-DD)");
            add_cmd->add_option("-t,--tags", tags, "Tags for the task");

            add_cmd->callback([=]() {
                handleAddCommand(name, status, priority, description, due_date, tags);
                });
        }

        // LIST command
        auto* list_cmd = app.add_subcommand("list", "List tasks");
        list_cmd->alias("ls");
        {
            std::string filter;
            list_cmd->add_option("filter", filter, "Filter tasks")
                ->check(CLI::IsMember({ "todo", "inprogress", "completed", "low", "medium", "high", "overdue" }));

            list_cmd->callback([=]() {
                handleListCommand(filter);
                });
        }

        // UPDATE command
        auto* update_cmd = app.add_subcommand("update", "Update an existing task");
        {
            int id;
            std::string name, status, priority;

            update_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);
            update_cmd->add_option("name", name, "New task name")->required();
            update_cmd->add_option("status", status, "New status")
                ->required()
                ->check(CLI::IsMember({ "todo", "inprogress", "completed" }));
            update_cmd->add_option("priority", priority, "New priority")
                ->required()
                ->check(CLI::IsMember({ "low", "medium", "high" }));

            update_cmd->callback([=]() {
                handleUpdateCommand(id, name, status, priority);
                });
        }

        // REMOVE command
        auto* remove_cmd = app.add_subcommand("remove", "Remove a task");
        remove_cmd->alias("rm");
        remove_cmd->alias("delete");
        {
            int id;
            remove_cmd->add_option("id", id, "Task ID to remove")->required()->check(CLI::PositiveNumber);

            remove_cmd->callback([=]() {
                handleRemoveCommand(id);
                });
        }

        // SEARCH command
        auto* search_cmd = app.add_subcommand("search", "Search tasks");
        search_cmd->alias("find");
        {
            std::string query;
            search_cmd->add_option("query", query, "Search query")->required();

            search_cmd->callback([=]() {
                handleSearchCommand(query);
                });
        }

        // DETAIL command
        auto* detail_cmd = app.add_subcommand("detail", "Show task details");
        detail_cmd->alias("show");
        detail_cmd->alias("info");
        {
            int id;
            detail_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);

            detail_cmd->callback([=]() {
                handleDetailCommand(id);
                });
        }

        // COMPLETE command
        auto* complete_cmd = app.add_subcommand("complete", "Mark task as completed");
        complete_cmd->alias("done");
        {
            int id;
            complete_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);

            complete_cmd->callback([=]() {
                handleCompleteCommand(id);
                });
        }

        // TAG command
        auto* tag_cmd = app.add_subcommand("tag", "Add tag to task");
        {
            int id;
            std::string tag;
            tag_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);
            tag_cmd->add_option("tag", tag, "Tag to add")->required();

            tag_cmd->callback([=]() {
                handleTagCommand(id, tag);
                });
        }

        // UNTAG command
        auto* untag_cmd = app.add_subcommand("untag", "Remove tag from task");
        {
            int id;
            std::string tag;
            untag_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);
            untag_cmd->add_option("tag", tag, "Tag to remove")->required();

            untag_cmd->callback([=]() {
                handleUntagCommand(id, tag);
                });
        }

        // DUE command
        auto* due_cmd = app.add_subcommand("due", "Set task due date");
        due_cmd->alias("deadline");
        {
            int id;
            std::string date;
            due_cmd->add_option("id", id, "Task ID")->required()->check(CLI::PositiveNumber);
            due_cmd->add_option("date", date, "Due date (YYYY-MM-DD)")->required();

            due_cmd->callback([=]() {
                handleDueDateCommand(id, date);
                });
        }

        // STATS command
        auto* stats_cmd = app.add_subcommand("stats", "Show task statistics");
        stats_cmd->alias("statistics");
        {
            stats_cmd->callback([=]() {
                handleStatsCommand();
                });
        }

        // OVERDUE command
        auto* overdue_cmd = app.add_subcommand("overdue", "Show overdue tasks");
        {
            overdue_cmd->callback([=]() {
                handleOverdueCommand();
                });
        }

        // Reload Tasks with the possibly changed data file
        if (config_.data_file != "data/data.json") {
            tasks_ = std::make_unique<Tasks>(config_.data_file);
        }

        // Parse and execute
        try {
            app.parse(argc, argv);
            return 0;
        }
        catch (const CLI::ParseError& e) {
            return app.exit(e);
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
