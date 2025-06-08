#include "TaskSearchIndex.hpp"
#include "benchmark.hpp"
#include "utils.hpp"
#include <algorithm>
#include <cctype>
#include <unordered_set>

void TaskSearchIndex::addTask(const Task& task) {
    // Index task name
    auto lowerName = Utils::toLowerCase(task.getName());
    indexString(lowerName, task);
    
    // Index description
    if (!task.getDescription().empty()) {
        auto lowerDesc = Utils::toLowerCase(task.getDescription());
        indexString(lowerDesc, task);
    }
    
    // Index tags
    for (const auto& tag : task.getTags()) {
        auto lowerTag = Utils::toLowerCase(tag);
        indexString(lowerTag, task);
    }
    
    // Index status and priority strings for search
    indexString(Utils::toLowerCase(task.getStatusString()), task);
    indexString(Utils::toLowerCase(task.getPriorityString()), task);
    
    ++total_indexed_tasks_;
}

void TaskSearchIndex::removeTask(const Task& task) {
    // Remove from name index
    auto lowerName = Utils::toLowerCase(task.getName());
    removeStringFromIndex(lowerName, task);
    
    // Remove from description index
    if (!task.getDescription().empty()) {
        auto lowerDesc = Utils::toLowerCase(task.getDescription());
        removeStringFromIndex(lowerDesc, task);
    }
    
    // Remove from tags index
    for (const auto& tag : task.getTags()) {
        auto lowerTag = Utils::toLowerCase(tag);
        removeStringFromIndex(lowerTag, task);
    }
    
    // Remove from status/priority index
    removeStringFromIndex(Utils::toLowerCase(task.getStatusString()), task);
    removeStringFromIndex(Utils::toLowerCase(task.getPriorityString()), task);
    
    if (total_indexed_tasks_ > 0) {
        --total_indexed_tasks_;
    }
}

void TaskSearchIndex::indexString(std::string_view str, const Task& task) {
    if (str.empty()) return;
    
    TrieNode* current = root_.get();
    
    // Index all prefixes of the string for comprehensive search
    for (size_t start = 0; start < str.length(); ++start) {
        current = root_.get();
        
        for (size_t i = start; i < str.length(); ++i) {
            char c = std::tolower(str[i]);
            
            if (current->children.find(c) == current->children.end()) {
                current->children[c] = std::make_unique<TrieNode>();
            }
            
            current = current->children[c].get();
        }
        
        // Add task reference to the final node
        auto& taskRefs = current->tasks;
        
        // Avoid duplicates (check if task already exists)
        bool exists = std::any_of(taskRefs.begin(), taskRefs.end(),
            [&task](const std::reference_wrapper<const Task>& ref) {
                return &ref.get() == &task;
            });
        
        if (!exists) {
            taskRefs.emplace_back(std::cref(task));
        }
    }
}

void TaskSearchIndex::removeStringFromIndex(std::string_view str, const Task& task) {
    if (str.empty()) return;
    
    // Remove task reference from all nodes where it was indexed
    for (size_t start = 0; start < str.length(); ++start) {
        TrieNode* current = root_.get();
        
        // Navigate to the target node
        for (size_t i = start; i < str.length(); ++i) {
            char c = std::tolower(str[i]);
            auto it = current->children.find(c);
            if (it == current->children.end()) {
                break; // Path doesn't exist
            }
            current = it->second.get();
        }
        
        // Remove task reference from this node
        auto& taskRefs = current->tasks;
        taskRefs.erase(
            std::remove_if(taskRefs.begin(), taskRefs.end(),
                [&task](const std::reference_wrapper<const Task>& ref) {
                    return &ref.get() == &task;
                }),
            taskRefs.end()
        );
    }
}

std::vector<std::reference_wrapper<const Task>>
TaskSearchIndex::searchPrefix(std::string_view prefix) const {
    BENCHMARK("Trie Prefix Search"); // Phase 2 optimization: Add benchmarking
    
    if (prefix.empty()) {
        return {};
    }
    
    auto lowerPrefix = Utils::toLowerCase(prefix);
    TrieNode* node = findNode(lowerPrefix);
    
    if (!node) {
        return {};
    }
    
    return collectTasks(node);
}

std::vector<std::reference_wrapper<const Task>>
TaskSearchIndex::searchSubstring(std::string_view substring) const {
    BENCHMARK("Trie Substring Search"); // Phase 2 optimization: Add benchmarking
    
    if (substring.empty()) {
        return {};
    }
    
    auto lowerSubstring = Utils::toLowerCase(substring);
    std::unordered_set<const Task*> uniqueTasks;
    std::vector<std::reference_wrapper<const Task>> results;
    
    // Collect all tasks from the entire trie and filter
    collectAllTasks(root_.get(), results);
    
    // Filter results to only include tasks that contain the substring
    std::vector<std::reference_wrapper<const Task>> filteredResults;
    filteredResults.reserve(results.size());
    
    for (const auto& taskRef : results) {
        const Task& task = taskRef.get();
        
        // Check if already processed (avoid duplicates)
        if (uniqueTasks.find(&task) != uniqueTasks.end()) {
            continue;
        }
        uniqueTasks.insert(&task);
        
        // Check if task contains substring in any searchable field
        auto lowerName = Utils::toLowerCase(task.getName());
        auto lowerDesc = Utils::toLowerCase(task.getDescription());
        
        bool matches = lowerName.find(lowerSubstring) != std::string::npos ||
                      lowerDesc.find(lowerSubstring) != std::string::npos;
        
        if (!matches) {
            // Check tags
            for (const auto& tag : task.getTags()) {
                auto lowerTag = Utils::toLowerCase(tag);
                if (lowerTag.find(lowerSubstring) != std::string::npos) {
                    matches = true;
                    break;
                }
            }
        }
        
        if (matches) {
            filteredResults.emplace_back(std::cref(task));
        }
    }
    
    return filteredResults;
}

TaskSearchIndex::TrieNode* TaskSearchIndex::findNode(std::string_view prefix) const {
    TrieNode* current = root_.get();
    
    for (char c : prefix) {
        char lowerC = std::tolower(c);
        auto it = current->children.find(lowerC);
        if (it == current->children.end()) {
            return nullptr;
        }
        current = it->second.get();
    }
    
    return current;
}

std::vector<std::reference_wrapper<const Task>> 
TaskSearchIndex::collectTasks(const TrieNode* node) const {
    std::vector<std::reference_wrapper<const Task>> results;
    std::unordered_set<const Task*> uniqueTasks;
    
    // Collect tasks from this node and all child nodes
    std::function<void(const TrieNode*)> collect = [&](const TrieNode* n) {
        if (!n) return;
        
        // Add tasks from current node
        for (const auto& taskRef : n->tasks) {
            const Task* taskPtr = &taskRef.get();
            if (uniqueTasks.find(taskPtr) == uniqueTasks.end()) {
                uniqueTasks.insert(taskPtr);
                results.emplace_back(taskRef);
            }
        }
        
        // Recursively collect from children
        for (const auto& [ch, child] : n->children) {
            collect(child.get());
        }
    };
    
    collect(node);
    return results;
}

void TaskSearchIndex::collectAllTasks(const TrieNode* node, 
                                     std::vector<std::reference_wrapper<const Task>>& results) const {
    if (!node) return;
    
    // Add tasks from current node
    for (const auto& taskRef : node->tasks) {
        results.emplace_back(taskRef);
    }
    
    // Recursively collect from children
    for (const auto& [ch, child] : node->children) {
        collectAllTasks(child.get(), results);
    }
}

void TaskSearchIndex::clear() {
    root_ = std::make_unique<TrieNode>();
    total_indexed_tasks_ = 0;
}

size_t TaskSearchIndex::getIndexMemoryUsage() const {
    size_t totalSize = sizeof(TaskSearchIndex);
    
    std::function<size_t(const TrieNode*)> calculateSize = [&](const TrieNode* node) -> size_t {
        if (!node) return 0;
        
        size_t nodeSize = sizeof(TrieNode);
        nodeSize += node->tasks.size() * sizeof(std::reference_wrapper<const Task>);
        nodeSize += node->children.size() * (sizeof(char) + sizeof(std::unique_ptr<TrieNode>));
        
        for (const auto& [ch, child] : node->children) {
            nodeSize += calculateSize(child.get());
        }
        
        return nodeSize;
    };
    
    return totalSize + calculateSize(root_.get());
}
