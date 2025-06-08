#!/bin/bash

# Build script for To-Do List Manager
echo "Starting build process..."

# Create obj directory if it doesn't exist
mkdir -p obj

# Compile source files
echo "Compiling Task.cpp..."
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude -c src/Task.cpp -o obj/Task.o

echo "Compiling Tasks.cpp..."
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude -c src/Tasks.cpp -o obj/Tasks.o

echo "Compiling utils.cpp..."
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude -c src/utils.cpp -o obj/utils.o

echo "Compiling main.cpp..."
g++ -std=c++20 -Wall -Wextra -O2 -Iinclude -c src/main.cpp -o obj/main.o

# Link all object files
echo "Linking executable..."
g++ obj/Task.o obj/Tasks.o obj/utils.o obj/main.o -o todo

echo "Build complete! Executable: ./todo"
