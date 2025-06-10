# 📋 To-Do List Manager v2.0 - Enhanced Edition

A modern, high-performance command-line task management application built with C++20/23. Features a custom CLI parser with no external dependencies, comprehensive task management capabilities, and optimized performance.

**✨ Status: Production Ready - CLI11-Free Version**

## 🚀 Features

- ✅ **Zero Dependencies**: Custom CLI parser, no external libraries required
- 🔍 **Advanced Search**: Fast task search with indexed lookups
- 📊 **Rich Statistics**: Comprehensive task analytics and progress tracking
- 🏷️ **Tag System**: Organize tasks with flexible tagging
- 📅 **Due Dates**: Set and track task deadlines with overdue detection
- 🎯 **Priority Management**: Three-level priority system (low, medium, high)
- 💾 **JSON Persistence**: Reliable data storage with automatic backups
- 🌈 **Colorized Output**: Beautiful terminal interface with emoji support
- ⚡ **Performance Optimized**: Parallel processing for large task collections

## 🛠️ Installation

### Quick Install (Recommended)
```bash
# Build and install system-wide (requires sudo)
sudo ./install.sh

# Or install for current user only
./install.sh
```

### Manual Installation
```bash
# Build the application
make

# Copy to your preferred location
sudo cp todo /usr/local/bin/
# or
cp todo ~/.local/bin/
```

## Features

- ✅ Add, update, remove, and search tasks
- 📊 Task prioritization (Low, Medium, High)
- 🔄 Task status tracking (To-Do, In Progress, Completed)
- 🎨 Colorized terminal output
- 💾 JSON-based data persistence
- 🔍 Flexible filtering and searching
- 📱 Clean command-line interface

## Project Structure

```
todo-app/
├── src/
│   ├── main.cpp          # Main application logic and command handlers
│   ├── Task.cpp          # Task class implementation
│   ├── Tasks.cpp         # Tasks container class implementation
│   └── utils.cpp         # Utility functions implementation
├── include/
│   ├── Task.hpp          # Task class header
│   ├── Tasks.hpp         # Tasks container class header
│   └── utils.hpp         # Utilities header
├── data/
│   └── data.json         # JSON file for persistent task storage
├── Makefile              # Build configuration
└── README.md             # This file
```

## Prerequisites

- C++20 compatible compiler (GCC 9+ or Clang 10+)
- nlohmann/json library (included in project)
- Make utility

## Installation

### Install nlohmann/json library

**Ubuntu/Debian:**
```bash
sudo apt-get update
sudo apt-get install nlohmann-json3-dev
```

**Arch Linux:**
```bash
sudo pacman -S nlohmann-json
```

**macOS (using Homebrew):**
```bash
brew install nlohmann-json
```

**Manual Installation:**
```bash
# Download the header file
wget https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp
sudo mkdir -p /usr/local/include/nlohmann
sudo cp json.hpp /usr/local/include/nlohmann/
```

### Build the Application

```bash
# Clone or navigate to the project directory
cd todo-app

# Build the application
make

# Or build with debug information
make debug
```

## Usage

### Basic Commands

1. **Add a new task:**
   ```bash
   ./todo add "Learn C++" 1 3
   ./todo add "Buy groceries"  # Default: status=1, priority=1
   ```

2. **Show all tasks:**
   ```bash
   ./todo show
   ```

3. **Show filtered tasks:**
   ```bash
   ./todo show -t    # To-Do tasks
   ./todo show -i    # In Progress tasks
   ./todo show -c    # Completed tasks
   ./todo show -l    # Low priority tasks
   ./todo show -m    # Medium priority tasks
   ./todo show -h    # High priority tasks
   ```

4. **Update a task:**
   ```bash
   ./todo update 1 "Updated task name" 2 3
   ```

5. **Remove a task:**
   ```bash
   ./todo remove 1
   ./todo rm 1      # Short form
   ```

6. **Search for tasks:**
   ```bash
   ./todo search "Learn"
   ```

7. **Get help:**
   ```bash
   ./todo --help
   ./todo -h
   ```

8. **Show version:**
   ```bash
   ./todo --version
   ./todo -v
   ```

### Status Codes
- **1** - To-Do (Red)
- **2** - In Progress (Yellow)
- **3** - Completed (Green)

### Priority Codes
- **1** - Low (Blue)
- **2** - Medium (Yellow)
- **3** - High (Red)

## Examples

```bash
# Add some sample tasks
./todo add "Learn C++23 features" 1 3
./todo add "Complete project documentation" 1 2
./todo add "Review code" 2 1

# Show all tasks
./todo show

# Show only high priority tasks
./todo show -h

# Update a task
./todo update 1 "Master C++23 features" 2 3

# Search for tasks
./todo search "C++"

# Remove a task
./todo rm 2
```

## Build Options

```bash
make          # Standard build
make debug    # Debug build with symbols
make clean    # Clean build files
make clean-all # Clean build files and data
make install  # Install to /usr/local/bin (requires sudo)
make uninstall # Remove from /usr/local/bin (requires sudo)
make run      # Build and run
make help     # Show available targets
```

## Testing & Demo

The project includes comprehensive testing and demo scripts:

```bash
# Run the interactive demo
./demo.sh

# Run comprehensive test suite
./test_suite.sh

# Manual build script (alternative to make)
./build.sh
```

## Data Persistence

Tasks are automatically saved to `data/data.json` in JSON format:

```json
{
    "nextId": 4,
    "tasks": [
        {
            "id": 1,
            "name": "Learn C++",
            "status": 1,
            "priority": 3
        }
    ]
}
```

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is open source and available under the MIT License.

## Version History

- **v1.0.0** - Initial release with basic task management features
