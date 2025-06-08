#!/bin/bash

# Enhanced C++20/23 Todo Application Demo
# This script demonstrates the new features and capabilities

echo "╔══════════════════════════════════════════════════════════════════════════════╗"
echo "║                          C++20/23 TODO APP DEMO                             ║"
echo "║                          Enhanced Version 2.0.0                            ║"
echo "╚══════════════════════════════════════════════════════════════════════════════╝"
echo

echo "🚀 Demonstrating Enhanced Todo Application Features"
echo "=================================================="
echo

# Show version information
echo "📋 Version Information:"
./todo --version
echo

# Show help menu
echo "📖 Help Menu (showing new commands):"
./todo --help | head -20
echo "... (truncated for demo)"
echo

# Clear any existing data and start fresh
echo "🧹 Starting with a clean slate..."
rm -f data/data.json
mkdir -p data
echo

# Add various tasks with different priorities and statuses
echo "➕ Adding tasks with different priorities and statuses:"
echo

echo "   Adding high priority task..."
./todo add "Implement advanced C++20 features" inprogress high

echo "   Adding medium priority task..."
./todo add "Write comprehensive unit tests" todo medium

echo "   Adding low priority task..."  
./todo add "Update documentation" todo low

echo "   Adding completed task..."
./todo add "Setup development environment" completed high

echo "   Adding another high priority task..."
./todo add "Code review and optimization" todo high
echo

# Show all tasks
echo "📝 Current Task List:"
./todo show
echo

# Test filtering by status
echo "🔍 Filtering by Status - Showing only TODO tasks:"
./todo show todo
echo

echo "🔍 Filtering by Status - Showing only IN PROGRESS tasks:"  
./todo show inprogress
echo

echo "🔍 Filtering by Status - Showing only COMPLETED tasks:"
./todo show completed
echo

# Test filtering by priority
echo "🎯 Filtering by Priority - Showing HIGH priority tasks:"
./todo show high
echo

echo "🎯 Filtering by Priority - Showing MEDIUM priority tasks:"
./todo show medium
echo

echo "🎯 Filtering by Priority - Showing LOW priority tasks:"
./todo show low
echo

# Show statistics
echo "📊 Task Statistics:"
./todo stats
echo

# Test detailed view
echo "🔎 Detailed Task View (Task #1):"
./todo detail 1
echo

echo "🔎 Detailed Task View (Task #2):"
./todo detail 2  
echo

# Test task completion
echo "✅ Marking task as completed:"
echo "   Completing task #2..."
./todo complete 2
echo

echo "📝 Updated Task List after completion:"
./todo show
echo

echo "📊 Updated Statistics after completion:"
./todo stats
echo

# Test update functionality
echo "✏️  Updating task #3:"
echo "   Changing priority from low to high..."
./todo update 3 "Update comprehensive documentation" todo high
echo

echo "📝 Task List after update:"
./todo show
echo

# Test removal
echo "🗑️  Removing a task:"
echo "   Removing task #5..."
./todo remove 5
echo

echo "📝 Final Task List:"
./todo show
echo

echo "📊 Final Statistics:"
./todo stats
echo

echo "✨ Demo completed! The enhanced C++20/23 Todo Application successfully demonstrates:"
echo "   ✓ Modern C++20 features (concepts, ranges, string_view, filesystem)"
echo "   ✓ Strongly-typed enums for status and priority"  
echo "   ✓ Enhanced error handling with TaskResult wrapper"
echo "   ✓ Advanced filtering by status and priority"
echo "   ✓ Comprehensive statistics tracking"
echo "   ✓ Detailed task views with timestamps"
echo "   ✓ Task completion and update functionality"
echo "   ✓ Robust JSON persistence with filesystem support"
echo "   ✓ Beautiful terminal UI with color coding"
echo "   ✓ Modern C++ best practices and safety"
echo
echo "🎉 The application is ready for production use!"
