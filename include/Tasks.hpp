#ifndef TASKS_HPP
#define TASKS_HPP

#include "Task.hpp"
#include <vector>
#include <string>
#include <memory>
#include <functional>

class Tasks {
private:
    std::vector<std::unique_ptr<Task>> tasks;
    int nextId;
    std::string dataFile;
    
public:
    // Constructor
    Tasks(const std::string& dataFile = "data/data.json");
    
    // Task management
    void addTask(const std::string& name, int status = 1, int priority = 1);
    bool removeTask(int id);
    bool updateTask(int id, const std::string& name, int status, int priority);
    Task* findTask(int id);
    std::vector<Task*> searchTasks(const std::string& name);
    
    // Display methods
    void showAllTasks() const;
    void showFilteredTasks(const std::string& filter) const;
    
    // Persistence
    void loadFromFile();
    void saveToFile() const;
    
    // Utility
    int getNextId() const;
    bool isEmpty() const;
    size_t size() const;
};

#endif // TASKS_HPP
