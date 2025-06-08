#include "Tasks.hpp"
#include "utils.hpp"
#include "CLI11.hpp"
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
    static constexpr const char* TAG_ADDED_MSG = "Tag added successfully!";
    static constexpr const char* TAG_REMOVED_MSG = "Tag removed successfully!";
    static constexpr const char* DUE_DATE_SET_MSG = "Due date set successfully!";
    static constexpr const char* INVALID_DATE_FORMAT_MSG = "Invalid date format. Use YYYY-MM-DD";

    void printResult(const TaskResult& result) {
        if (result.success) {
            std::cout << Utils::GREEN << result.message << Utils::RESET << std::endl;
        }
        else {
            std::cout << Utils::RED << "Error: " << result.message << Utils::RESET << std::endl;
        }
    }

    // Helper method to validate task ID and return task pointer
    Task* validateAndFindTask(int id) {
        auto task = tasks.findTask(id);
        if (!task) {
            std::cout << Utils::RED << std::format(TASK_NOT_FOUND_MSG, id) << Utils::RESET << std::endl;
        }
        return task;
    }

public:
    TodoApp() : tasks("data/data.json") {}

    int run(int argc, char* argv[]) { // Changed return type to int
        CLI::App app{ "To-Do List Manager v2.0 - A powerful, modern task management system built with C++20/23\n\n" };
        app.require_subcommand(0, 1);

        // Enhance help message with a footer and --help-all flag
        app.footer("Examples:\n"
            "  todo add \"Buy groceries\" -p high\n"
            "  todo show --filter completed\n\n"
            "Run 'todo <subcommand> --help' for more information on a specific command.");
        app.set_help_all_flag("--help-all", "Show help for all commands and subcommands");

        // Global options
        app.add_flag_function("--version,-v", [](int /*count*/) {
            std::cout << "To-Do List Manager v2.0.0 Enhanced Edition" << std::endl;
            throw CLI::Success(); // Exit after printing version
            }, "Show version information")->group("Global Options");

        // Default behavior when no subcommand is provided: print help
        app.callback([&app]() {
            std::cout << app.help() << std::endl;
            throw CLI::Success(); // Exit after printing help
            });

        setupAddCommand(app);
        setupShowCommand(app);
        setupUpdateCommand(app);
        setupRemoveCommand(app);
        setupSearchCommand(app);
        setupDetailCommand(app);
        setupOverdueCommand(app);
        setupStatsCommand(app);
        setupTagCommand(app);
        setupUntagCommand(app);
        setupDueCommand(app);
        setupCompleteCommand(app);

        try {
            app.parse(argc, argv);
        }
        catch (const CLI::Success&) {
            return 0; // Successful exit (e.g., --version, --help, or main app callback)
        }
        catch (const CLI::ParseError& e) {
            return app.exit(e); // Exit with code from CLI11
        }

        return 0; // Success if a subcommand ran
    }

private:
    void setupAddCommand(CLI::App& app) {
        auto* add_cmd = app.add_subcommand("add", "Add a new task to your todo list");

        // Use member variables to properly handle CLI11 state
        static std::string name;
        static std::string status_str = "todo";
        static std::string priority_str = "low";

        add_cmd->add_option("name", name, "Name or description of the task")->required();
        add_cmd->add_option("-s,--status", status_str, "Initial task status")
            ->check(CLI::IsMember({ "todo", "inprogress", "completed" }));
        add_cmd->add_option("-p,--priority", priority_str, "Task priority level")
            ->check(CLI::IsMember({ "low", "medium", "high" }));

        add_cmd->callback([this]() {
            handleAdd(name, status_str, priority_str);
            // Reset to defaults for next use
            name.clear();
            status_str = "todo";
            priority_str = "low";
            });
    }

    void setupShowCommand(CLI::App& app) {
        auto* show_cmd = app.add_subcommand("show", "Display tasks with optional filtering");

        static std::string filter;
        show_cmd->add_option("filter", filter, "Filter by status (todo/inprogress/completed) or priority (low/medium/high)")
            ->check(CLI::IsMember({ "todo", "inprogress", "completed", "low", "medium", "high" }));

        show_cmd->callback([this]() {
            handleShow(filter);
            filter.clear(); // Clear for next use
            });
    }

    void setupUpdateCommand(CLI::App& app) {
        auto* update_cmd = app.add_subcommand("update", "Update an existing task's properties");

        static int id;
        static std::string name;
        static std::string status_str;
        static std::string priority_str;

        update_cmd->add_option("id", id, "ID of the task to update")->required();
        update_cmd->add_option("name", name, "New name/description for the task")->required();
        update_cmd->add_option("status", status_str, "New status for the task")
            ->required()
            ->check(CLI::IsMember({ "todo", "inprogress", "completed" }));
        update_cmd->add_option("priority", priority_str, "New priority level for the task")
            ->required()
            ->check(CLI::IsMember({ "low", "medium", "high" }));

        update_cmd->callback([this]() {
            handleUpdate(id, name, status_str, priority_str);
            // Clear static variables for next use
            name.clear();
            status_str.clear();
            priority_str.clear();
            });
    }

    void setupRemoveCommand(CLI::App& app) {
        auto* remove_cmd = app.add_subcommand("remove", "Remove a task from your todo list");
        remove_cmd->alias("rm");

        int id;
        remove_cmd->add_option("id", id, "ID of the task to remove")->required();

        remove_cmd->callback([this, &id]() {
            handleRemove(id);
            });
    }

    void setupSearchCommand(CLI::App& app) {
        auto* search_cmd = app.add_subcommand("search", "Search tasks by name, description, or tags");

        std::string query;
        search_cmd->add_option("query", query, "Search term to look for in task names and descriptions")->required();

        search_cmd->callback([this, &query]() {
            handleSearch(query);
            });
    }

    void setupDetailCommand(CLI::App& app) {
        auto* detail_cmd = app.add_subcommand("detail", "Show detailed information for a specific task");

        int id;
        detail_cmd->add_option("id", id, "ID of the task to show details for")->required();

        detail_cmd->callback([this, &id]() {
            handleDetail(id);
            });
    }

    void setupOverdueCommand(CLI::App& app) {
        auto* overdue_cmd = app.add_subcommand("overdue", "Show all tasks that are past their due date");

        overdue_cmd->callback([this]() {
            handleOverdue();
            });
    }

    void setupStatsCommand(CLI::App& app) {
        auto* stats_cmd = app.add_subcommand("stats", "Display comprehensive task statistics and analytics");

        stats_cmd->callback([this]() {
            handleStats();
            });
    }

    void setupTagCommand(CLI::App& app) {
        auto* tag_cmd = app.add_subcommand("tag", "Add a tag to an existing task for better organization");

        int id;
        std::string tag;
        tag_cmd->add_option("id", id, "ID of the task to tag")->required();
        tag_cmd->add_option("tag", tag, "Tag name to add to the task")->required();

        tag_cmd->callback([this, &id, &tag]() {
            handleTag(id, tag);
            });
    }

    void setupUntagCommand(CLI::App& app) {
        auto* untag_cmd = app.add_subcommand("untag", "Remove a tag from an existing task");

        int id;
        std::string tag;
        untag_cmd->add_option("id", id, "ID of the task to remove tag from")->required();
        untag_cmd->add_option("tag", tag, "Tag name to remove from the task")->required();

        untag_cmd->callback([this, &id, &tag]() {
            handleUntag(id, tag);
            });
    }

    void setupDueCommand(CLI::App& app) {
        auto* due_cmd = app.add_subcommand("due", "Set or update the due date for a specific task");

        int id;
        std::string date;
        due_cmd->add_option("id", id, "ID of the task to set due date for")->required();
        due_cmd->add_option("date", date, "Due date in YYYY-MM-DD format (e.g., 2025-12-31)")->required();

        due_cmd->callback([this, &id, &date]() {
            handleDue(id, date);
            });
    }

    void setupCompleteCommand(CLI::App& app) {
        auto* complete_cmd = app.add_subcommand("complete", "Mark a task as completed");

        int id;
        complete_cmd->add_option("id", id, "ID of the task to mark as completed")->required();

        complete_cmd->callback([this, &id]() {
            handleComplete(id);
            });
    }

    // Modernized handler methods
    void handleAdd(const std::string& name, const std::string& status_str, const std::string& priority_str) {
        try {
            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            auto result = tasks.addTask(name, status, priority);
            printResult(result);
        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleShow(const std::string& filter) {
        if (filter.empty()) {
            tasks.showAllTasks();
            return;
        }

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

        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleUpdate(int id, const std::string& name, const std::string& status_str, const std::string& priority_str) {
        try {
            TaskStatus status = Utils::parseTaskStatus(status_str);
            TaskPriority priority = Utils::parseTaskPriority(priority_str);

            auto result = tasks.updateTask(id, name, status, priority);
            printResult(result);

        }
        catch (const std::exception& e) {
            std::cout << Utils::RED << "Error: " << e.what() << Utils::RESET << std::endl;
        }
    }

    void handleRemove(int id) {
        auto result = tasks.removeTask(id);
        printResult(result);
    }

    void handleSearch(const std::string& query) {
        auto results = tasks.searchTasks(query);

        if (results.empty()) {
            std::cout << Utils::YELLOW << "No tasks found matching: " << query << Utils::RESET << std::endl;
            return;
        }

        tasks.displayTaskList(results, std::format("Search results for: {}", query));
    }

    void handleDetail(int id) {
        tasks.showTaskDetails(id);
    }

    void handleOverdue() {
        tasks.showOverdueTasks();
    }

    void handleStats() {
        tasks.showStatistics();
    }

    void handleTag(int id, const std::string& tag) {
        auto task = validateAndFindTask(id);
        if (!task) return;

        task->addTag(tag);
        tasks.save();
        std::cout << Utils::GREEN << TAG_ADDED_MSG << Utils::RESET << std::endl;
    }

    void handleUntag(int id, const std::string& tag) {
        auto task = validateAndFindTask(id);
        if (!task) return;

        task->removeTag(tag);
        tasks.save();
        std::cout << Utils::GREEN << TAG_REMOVED_MSG << Utils::RESET << std::endl;
    }

    void handleDue(int id, const std::string& date) {
        auto task = validateAndFindTask(id);
        if (!task) return;

        auto dueDate = Utils::parseDate(date);
        if (!dueDate) {
            std::cout << Utils::RED << INVALID_DATE_FORMAT_MSG << Utils::RESET << std::endl;
            return;
        }

        task->setDueDate(dueDate);
        tasks.save();
        std::cout << Utils::GREEN << DUE_DATE_SET_MSG << Utils::RESET << std::endl;
    }

    void handleComplete(int id) {
        if (auto task = tasks.findTask(id)) {
            task->markCompleted();
            tasks.save();
            std::cout << Utils::GREEN << "Task marked as completed!" << Utils::RESET << std::endl;
        }
        else {
            std::cout << Utils::RED << "Task with ID " << id << " not found!" << Utils::RESET << std::endl;
        }
    }
};

int main(int argc, char* argv[]) {
    TodoApp app_instance;
    try {
        return app_instance.run(argc, argv); // Return exit code from run
    }
    catch (const std::exception& e) {
        std::cerr << "An unexpected error occurred: " << e.what() << std::endl;
        return 1;
    }
}
