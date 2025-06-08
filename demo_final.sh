#!/bin/bash

# Comprehensive C++20/23 Todo Application Demo
# Demonstrates all enhanced features and capabilities

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                    🚀 C++20/23 TODO APPLICATION DEMO 🚀                     ║"
echo "║                           Enhanced Version 2.0.0                            ║"
echo "║                     Modern C++ Features & Best Practices                    ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo

# Function to pause and wait for user input
pause() {
    echo -e "\n🔄 Press Enter to continue..."
    read -r
}

# Function to run command with header
run_command() {
    echo -e "\n📋 $1"
    echo "$(printf '=%.0s' {1..60})"
    shift
    "$@" 2>&1
}

echo "🎯 This demo showcases the enhanced C++20/23 Todo Application featuring:"
echo "   ✅ Modern C++20 features (concepts, ranges, string_view, filesystem)"
echo "   ✅ Strongly-typed enums with enhanced type safety"
echo "   ✅ TaskResult wrapper for robust error handling"  
echo "   ✅ Advanced filtering and search capabilities"
echo "   ✅ Comprehensive statistics and analytics"
echo "   ✅ Beautiful terminal UI with color coding"
echo "   ✅ JSON persistence with backup support"
echo "   ✅ Enhanced task management features"

pause

# Show version and help
run_command "Version Information" ./todo --version

run_command "Help System Overview" ./todo --help

pause

# Clear existing data for clean demo
echo "🧹 Setting up clean environment..."
rm -f data/data.json
mkdir -p data

# Demonstrate task creation with different priorities and statuses
run_command "Adding Tasks with Various Priorities and Statuses" \
echo "Creating sample tasks..."

echo "➕ Adding high priority in-progress task..."
./todo add "Implement C++20 coroutines" inprogress high 2>&1

echo "➕ Adding medium priority todo task..."
./todo add "Write comprehensive tests" todo medium 2>&1

echo "➕ Adding low priority task..."
./todo add "Update README documentation" todo low 2>&1

echo "➕ Adding completed high priority task..."
./todo add "Setup CI/CD pipeline" completed high 2>&1

echo "➕ Adding another high priority task..."
./todo add "Performance optimization" todo high 2>&1

echo "➕ Adding medium priority task..."
./todo add "Code review automation" todo medium 2>&1

pause

# Show all tasks
run_command "Current Task Overview" ./todo show

pause

# Demonstrate filtering by status
run_command "Filtering by Status: TODO Tasks" ./todo show todo

run_command "Filtering by Status: IN PROGRESS Tasks" ./todo show inprogress

run_command "Filtering by Status: COMPLETED Tasks" ./todo show completed

pause

# Demonstrate filtering by priority
run_command "Filtering by Priority: HIGH Priority Tasks" ./todo show high

run_command "Filtering by Priority: MEDIUM Priority Tasks" ./todo show medium

run_command "Filtering by Priority: LOW Priority Tasks" ./todo show low

pause

# Show statistics
run_command "Task Statistics and Analytics" ./todo stats

pause

# Demonstrate task completion
echo "✅ Demonstrating Task Completion"
echo "$(printf '=%.0s' {1..60})"
echo "Marking task #2 as completed..."
./todo complete 2 2>&1

echo -e "\nUpdated task list after completion:"
./todo show 2>&1

pause

# Demonstrate task updates
echo "✏️  Demonstrating Task Updates"
echo "$(printf '=%.0s' {1..60})"
echo "Updating task #3 priority from low to high..."
./todo update 3 "Update comprehensive README documentation" todo high 2>&1

echo -e "\nTask list after update:"
./todo show 2>&1

pause

# Show updated statistics
run_command "Updated Statistics After Changes" ./todo stats

pause

# Demonstrate task removal
echo "🗑️  Demonstrating Task Removal"
echo "$(printf '=%.0s' {1..60})"
echo "Removing task #6..."
./todo remove 6 2>&1

echo -e "\nFinal task list:"
./todo show 2>&1

pause

# Final statistics
run_command "Final Statistics Summary" ./todo stats

echo
echo "🎉 Demo Complete! Key Features Demonstrated:"
echo "$(printf '=%.0s' {1..60})"
echo "✅ Task Management: Add, Update, Complete, Remove"
echo "✅ Advanced Filtering: By Status (todo/inprogress/completed)"  
echo "✅ Priority Filtering: By Priority (high/medium/low)"
echo "✅ Statistics Dashboard: Real-time task analytics"
echo "✅ Modern C++20 Features: Strong typing, error handling"
echo "✅ Beautiful UI: Color-coded, organized display"
echo "✅ Data Persistence: JSON-based storage"
echo "✅ Enhanced Error Handling: TaskResult wrapper pattern"
echo "✅ Type Safety: Strongly-typed enums for status/priority"
echo "✅ Modern C++ Best Practices: RAII, concepts, ranges"

echo
echo "🚀 Technical Highlights:"
echo "   🔧 Built with C++20/23 standard features"
echo "   🛡️  Memory-safe with smart pointers and RAII"
echo "   ⚡ High-performance with move semantics"
echo "   🎨 Beautiful terminal UI with Unicode support"
echo "   🔍 Advanced filtering and search capabilities"
echo "   📊 Comprehensive statistics and analytics"
echo "   💾 Robust JSON persistence layer"
echo "   🧪 Comprehensive error handling"

echo  
echo "✨ The Enhanced C++20/23 Todo Application is ready for production use!"
echo "   Perfect for developers who appreciate modern C++ and robust software design."
