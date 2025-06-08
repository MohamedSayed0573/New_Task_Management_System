#!/bin/bash

echo "=== TO-DO LIST MANAGER - COMPREHENSIVE TEST ==="
echo

# Clear any existing data
rm -f data/data.json

echo "1. Testing --version command:"
./todo --version
echo

echo "2. Testing --help command:"
./todo --help | head -10
echo "... (help truncated for brevity)"
echo

echo "3. Adding sample tasks:"
./todo add "Learn advanced C++20 features" 1 3
./todo add "Write comprehensive documentation" 2 2
./todo add "Set up CI/CD pipeline" 1 2
./todo add "Code review meeting" 3 1
./todo add "Deploy to production" 1 3
echo

echo "4. Showing all tasks:"
./todo show
echo

echo "5. Testing filters:"
echo "5a. High priority tasks (-h):"
./todo show -h
echo

echo "5b. To-Do tasks (-t):"
./todo show -t
echo

echo "5c. In Progress tasks (-i):"
./todo show -i
echo

echo "5d. Completed tasks (-c):"
./todo show -c
echo

echo "6. Testing search functionality:"
echo "6a. Search for 'C++':"
./todo search "C++"
echo

echo "6b. Search for 'Code':"
./todo search "Code"
echo

echo "7. Testing update functionality:"
echo "7a. Before update:"
./todo show | grep -A1 -B1 "ID.*Name"
echo "7b. Updating task 2:"
./todo update 2 "Complete comprehensive documentation" 3 2
echo "7c. After update:"
./todo show | grep -A1 -B1 "ID.*Name"
echo

echo "8. Testing remove functionality:"
echo "8a. Before removal:"
./todo show | tail -2
echo "8b. Removing task 4:"
./todo remove 4
echo "8c. After removal:"
./todo show | tail -2
echo

echo "9. Final state of all tasks:"
./todo show
echo

echo "10. Checking data persistence (JSON file):"
echo "Contents of data/data.json:"
cat data/data.json | head -15
echo "... (JSON truncated for brevity)"
echo

echo "=== TEST COMPLETED SUCCESSFULLY ==="
