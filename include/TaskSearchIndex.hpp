/**
 * @file TaskSearchIndex.hpp
 * @brief Advanced search index implementation using trie data structure
 *
 * This header provides a high-performance search index for tasks using a trie (prefix tree)
 * data structure. Enables fast prefix-based searches and efficient task indexing.
 *
 * Features:
 * - O(m) prefix search where m is the length of the query
 * - Efficient memory usage with shared prefixes
 * - Support for both prefix and substring searches
 * - Task reference storage to avoid copying
 */

#ifndef TASK_SEARCH_INDEX_HPP
#define TASK_SEARCH_INDEX_HPP

#include "Task.hpp"
#include <unordered_map>
#include <memory>
#include <vector>
#include <functional>
#include <string_view>

 /**
  * @class TaskSearchIndex
  * @brief Phase 2 optimization: Advanced search index with trie structure
  *
  * Implements a trie-based search index for fast task searching. The trie allows
  * for efficient prefix-based searches and can handle large numbers of tasks
  * with good performance characteristics.
  *
  * The index stores references to tasks rather than copies, ensuring memory
  * efficiency and consistency with the main task storage.
  */
class TaskSearchIndex {
public:
    /**
     * @struct TrieNode
     * @brief Node in the trie data structure
     *
     * Each node represents a character in the search index and contains:
     * - Children nodes for the next characters
     * - A list of tasks that contain the path to this node as a prefix
     */
    struct TrieNode {
        std::unordered_map<char, std::unique_ptr<TrieNode>> children;    ///< Child nodes for each character
        std::vector<std::reference_wrapper<const Task>> tasks;           ///< Tasks containing this prefix

        TrieNode() = default;
        ~TrieNode() = default;

        // Non-copyable due to unique_ptr members, but movable for efficiency
        TrieNode(const TrieNode&) = delete;
        TrieNode& operator=(const TrieNode&) = delete;
        TrieNode(TrieNode&&) = default;
        TrieNode& operator=(TrieNode&&) = default;
    };

private:
    std::unique_ptr<TrieNode> root_;        ///< Root node of the trie
    size_t total_indexed_tasks_ = 0;        ///< Total number of tasks in the index

public:
    /**
     * @brief Construct empty search index
     */
    TaskSearchIndex() : root_(std::make_unique<TrieNode>()) {}

    // =====================
    // Index Management
    // =====================

    /**
     * @brief Add a task to the search index
     * @param task Task to index
     *
     * Indexes all searchable content of the task including:
     * - Task name (lowercase)
     * - Description (lowercase)
     * - All tags (lowercase)
     *
     * Creates trie paths for efficient prefix searching.
     */
    void addTask(const Task& task);

    /**
     * @brief Remove a task from the search index
     * @param task Task to remove
     *
     * Removes the task from all trie nodes where it was indexed.
     * Does not delete empty nodes for performance reasons.
     */
    void removeTask(const Task& task);

    /**
     * @brief Clear the entire search index
     *
     * Removes all tasks and resets the trie to empty state.
     * Useful for bulk index rebuilding.
     */
    void clear();

    // ==================
    // Search Operations
    // ==================

    /**
     * @brief Search for tasks with content matching a prefix
     * @param prefix Prefix string to search for
     * @return Vector of task references matching the prefix
     *
     * Performs efficient O(m) prefix search where m is the length of the prefix.
     * Returns all tasks that contain words starting with the given prefix.
     */
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        searchPrefix(std::string_view prefix) const;

    /**
     * @brief Search for tasks containing a substring (slower than prefix search)
     * @param substring Substring to search for
     * @return Vector of task references containing the substring
     *
     * Performs substring search which is slower than prefix search but more flexible.
     * Useful when you need to find text anywhere within task content.
     */
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        searchSubstring(std::string_view substring) const;

    // ===================
    // Index Statistics
    // ===================

    /**
     * @brief Get total number of indexed tasks
     * @return Number of tasks in the index
     */
    [[nodiscard]] size_t getTotalIndexedTasks() const noexcept { return total_indexed_tasks_; }

    /**
     * @brief Estimate memory usage of the index
     * @return Approximate memory usage in bytes
     *
     * Provides an estimate of the memory consumed by the trie structure.
     * Useful for monitoring and optimization purposes.
     */
    [[nodiscard]] size_t getIndexMemoryUsage() const;

private:
    // ===================
    // Internal Helpers
    // ===================

    /**
     * @brief Index a string by creating trie paths
     * @param str String to index
     * @param task Task associated with this string
     *
     * Creates trie paths for all prefixes of the string and associates
     * the task with each relevant node.
     */
    void indexString(std::string_view str, const Task& task);

    /**
     * @brief Find trie node for a given prefix
     * @param prefix Prefix to search for
     * @return Pointer to the trie node or nullptr if not found
     */
    [[nodiscard]] TrieNode* findNode(std::string_view prefix) const;

    /**
     * @brief Collect all tasks from a subtree
     * @param node Root node of subtree
     * @return Vector of all tasks in the subtree
     *
     * Recursively collects all tasks from a node and its descendants.
     * Used for prefix search result collection.
     */
    [[nodiscard]] std::vector<std::reference_wrapper<const Task>>
        collectTasks(const TrieNode* node) const;

    /**
     * @brief Remove a task from trie paths for a specific string
     * @param str String that was indexed
     * @param task Task to remove
     *
     * Removes task references from all trie nodes associated with the string.
     */
    void removeStringFromIndex(std::string_view str, const Task& task);

    /**
     * @brief Recursively collect all tasks from a subtree
     * @param node Current node
     * @param results Output vector to collect results
     *
     * Helper function for collectTasks that performs the actual recursion.
     */
    void collectAllTasks(const TrieNode* node,
        std::vector<std::reference_wrapper<const Task>>& results) const;
};

#endif // TASK_SEARCH_INDEX_HPP
