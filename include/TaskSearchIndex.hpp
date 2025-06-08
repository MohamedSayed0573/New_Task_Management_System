#ifndef TASK_SEARCH_INDEX_HPP
#define TASK_SEARCH_INDEX_HPP

#include "Task.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <string_view>

// Phase 2 optimization: Advanced search index with trie structure
class TaskSearchIndex {
public:
    struct TrieNode {
        std::unordered_map<char, std::unique_ptr<TrieNode>> children;
        std::vector<std::reference_wrapper<const Task>> tasks;

        TrieNode() = default;
        ~TrieNode() = default;

        // Non-copyable, movable
        TrieNode(const TrieNode&) = delete;
        TrieNode& operator=(const TrieNode&) = delete;
        TrieNode(TrieNode&&) = default;
        TrieNode& operator=(TrieNode&&) = default;
    };

private:
    std::unique_ptr<TrieNode> root_;
    size_t total_indexed_tasks_ = 0;

public:
    TaskSearchIndex() : root_(std::make_unique<TrieNode>()) {}

    // Index a task by all its searchable content
    void addTask(const Task& task);

    // Remove a task from the index
    void removeTask(const Task& task);

    // Search for tasks matching a prefix
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        searchPrefix(std::string_view prefix) const;

    // Search for tasks containing a substring (slower than prefix search)
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        searchSubstring(std::string_view substring) const;

    // Clear the entire index
    void clear();

    // Get statistics
    [[nodiscard]] size_t getTotalIndexedTasks() const noexcept { return total_indexed_tasks_; }
    [[nodiscard]] size_t getIndexMemoryUsage() const;

private:
    void indexString(std::string_view str, const Task& task);
    [[nodiscard]] TrieNode* findNode(std::string_view prefix) const;
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        collectTasks(const TrieNode* node) const;

    void removeStringFromIndex(std::string_view str, const Task& task);
    void collectAllTasks(const TrieNode* node,
        std::vector<std::reference_wrapper<const Task>>& results) const;
};

#endif // TASK_SEARCH_INDEX_HPP
