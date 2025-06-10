#ifndef TASK_FILTERS_HPP
#define TASK_FILTERS_HPP

#include "Task.hpp"
#include <ranges>
#include <concepts>

// Phase 2 optimization: Modern C++ ranges and concepts for efficient filtering
namespace TaskFilters {

    // Concepts for type safety
    template<typename T>
    concept TaskContainer = requires(T & container) {
        typename T::value_type;
        requires std::same_as<typename T::value_type, std::unique_ptr<Task>> ||
        std::same_as<typename T::value_type, Task*>;
        { container.begin() } -> std::forward_iterator;
        { container.end() } -> std::forward_iterator;
        { container.size() } -> std::convertible_to<size_t>;
    };

    // Helper function to get task reference from container element
    template<typename T>
    const Task& getTaskRef(const T& task_ptr) {
        if constexpr (std::is_pointer_v<std::decay_t<T>>) {
            return *task_ptr;
        }
        else {
            return *task_ptr;  // unique_ptr case
        }
    }

    // High-priority incomplete tasks using ranges and views
    template<TaskContainer Container>
    auto getHighPriorityIncompleteTasks(const Container& tasks) {
        return tasks
            | std::views::filter([](const auto& task_ptr) {
            const Task& task = getTaskRef(task_ptr);
            return task.getPriority() == TaskPriority::HIGH &&
                task.getStatus() != TaskStatus::COMPLETED;
                });
    }

    // Overdue tasks with critical priority
    template<TaskContainer Container>
    auto getCriticalOverdueTasks(const Container& tasks) {
        return tasks
            | std::views::filter([](const auto& task_ptr) {
            const Task& task = getTaskRef(task_ptr);
            return task.isOverdue() && task.getPriority() == TaskPriority::HIGH;
                });
    }

    // Tasks by multiple criteria with lazy evaluation
    template<TaskContainer Container>
    auto getTasksByMultipleCriteria(const Container& tasks,
        TaskStatus status,
        TaskPriority min_priority) {
        return tasks
            | std::views::filter([status, min_priority](const auto& task_ptr) {
            const Task& task = getTaskRef(task_ptr);
            return task.getStatus() == status &&
                static_cast<int>(task.getPriority()) >= static_cast<int>(min_priority);
                });
    }

    // Completion rate calculation using ranges
    template<TaskContainer Container>
    constexpr double calculateCompletionRate(const Container& tasks) {
        if (std::ranges::empty(tasks)) return 0.0;

        auto completed_count = std::ranges::count_if(tasks, [](const auto& task_ptr) {
            const Task& task = getTaskRef(task_ptr);
            return task.getStatus() == TaskStatus::COMPLETED;
            });

        return static_cast<double>(completed_count) / std::ranges::size(tasks);
    }

    // Task statistics using ranges algorithms
    template<TaskContainer Container>
    struct TaskMetrics {
        size_t total_tasks;
        size_t completed_tasks;
        size_t high_priority_tasks;
        size_t overdue_tasks;
        double completion_rate;

        static TaskMetrics calculate(const Container& tasks) {
            TaskMetrics metrics{};

            metrics.total_tasks = std::ranges::size(tasks);

            if (metrics.total_tasks == 0) {
                return metrics;
            }

            metrics.completed_tasks = std::ranges::count_if(tasks, [](const auto& task_ptr) {
                const Task& task = getTaskRef(task_ptr);
                return task.getStatus() == TaskStatus::COMPLETED;
                });

            metrics.high_priority_tasks = std::ranges::count_if(tasks, [](const auto& task_ptr) {
                const Task& task = getTaskRef(task_ptr);
                return task.getPriority() == TaskPriority::HIGH;
                });

            metrics.overdue_tasks = std::ranges::count_if(tasks, [](const auto& task_ptr) {
                const Task& task = getTaskRef(task_ptr);
                return task.isOverdue();
                });

            metrics.completion_rate = static_cast<double>(metrics.completed_tasks) / metrics.total_tasks;

            return metrics;
        }
    };

} // namespace TaskFilters

#endif // TASK_FILTERS_HPP
