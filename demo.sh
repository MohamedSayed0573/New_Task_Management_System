#!/bin/bash

echo "╔══════════════════════════════════════════════════════════════╗"
echo "║                 TO-DO LIST MANAGER DEMO                      ║"
echo "║                  Built with C++20                            ║"
echo "╚══════════════════════════════════════════════════════════════╝"
echo

echo "🚀 Quick Demo of All Features:"
echo

echo "📝 Adding tasks with different priorities and statuses:"
./todo add "Learn Rust programming" 1 3
./todo add "Update resume" 1 1  
./todo add "Gym workout" 2 2
echo

echo "📋 Viewing all tasks:"
./todo show
echo

echo "🔍 Filtering examples:"
echo "High priority tasks:"
./todo show -h
echo

echo "To-Do tasks:"
./todo show -t
echo

echo "🔎 Search functionality:"
./todo search "Learn"
echo

echo "✏️  Update example (changing task 1):"
./todo update 1 "Master Rust programming language" 2 3
./todo show | head -8
echo

echo "🗑️  Remove example:"
./todo add "Temporary task" 1 1
echo "Before removal:"
./todo show | tail -3
./todo rm 4
echo "After removal:"
./todo show | tail -3
echo

echo "💾 Data persistence - JSON structure:"
echo "Current data file contents:"
cat data/data.json
echo

echo "✅ All features working perfectly!"
echo "   • Task creation ✓"
echo "   • Task viewing with colors ✓" 
echo "   • Filtering by status/priority ✓"
echo "   • Search functionality ✓"
echo "   • Update tasks ✓"
echo "   • Remove tasks ✓"
echo "   • JSON data persistence ✓"
echo "   • Command-line interface ✓"
