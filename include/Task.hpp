#ifndef TASK_HPP
#define TASK_HPP

#include <string>
#include "json.hpp"

class Task {
private:
    int id;
    std::string name;
    int status;    // 1=To-Do, 2=In Progress, 3=Completed
    int priority;  // 1=Low, 2=Medium, 3=High

public:
    // Constructor
    Task(int id, const std::string& name, int status = 1, int priority = 1);
    
    // Getters
    int getId() const;
    std::string getName() const;
    int getStatus() const;
    int getPriority() const;
    
    // Setters
    void setName(const std::string& name);
    void setStatus(int status);
    void setPriority(int priority);
    
    // Status helper methods
    std::string getStatusString() const;
    std::string getPriorityString() const;
    
    // JSON serialization
    nlohmann::json toJson() const;
    static Task fromJson(const nlohmann::json& j);
    
    // Display
    std::string toString() const;
};

#endif // TASK_HPP
