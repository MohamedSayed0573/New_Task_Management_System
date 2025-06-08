# Makefile for To-Do List Manager - Enhanced with Phase 1 optimizations
CXX = g++
CXXFLAGS_BASE = -std=c++23 -Wall -Wextra -Iinclude
CXXFLAGS_DEBUG = $(CXXFLAGS_BASE) -g -O0 -DDEBUG -fsanitize=address,undefined
CXXFLAGS_RELEASE = $(CXXFLAGS_BASE) -O3 -DNDEBUG -march=native -flto -ffast-math
CXXFLAGS_PROFILE = $(CXXFLAGS_RELEASE) -pg -fno-omit-frame-pointer

# Link Time Optimization for release builds
LDFLAGS_RELEASE = -flto -Wl,--gc-sections -s
LDFLAGS_DEBUG = -fsanitize=address,undefined
LDFLAGS_PROFILE = $(LDFLAGS_RELEASE) -pg

# Select build type (default: release)
BUILD_TYPE ?= release
ifeq ($(BUILD_TYPE),debug)
    CXXFLAGS = $(CXXFLAGS_DEBUG)
    LDFLAGS = $(LDFLAGS_DEBUG)
else ifeq ($(BUILD_TYPE),profile)
    CXXFLAGS = $(CXXFLAGS_PROFILE)
    LDFLAGS = $(LDFLAGS_PROFILE)
else
    CXXFLAGS = $(CXXFLAGS_RELEASE)
    LDFLAGS = $(LDFLAGS_RELEASE)
endif

# Parallel compilation
MAKEFLAGS += -j$(shell nproc)

# Directories
SRCDIR = src
INCDIR = include
OBJDIR = obj
DATADIR = data

# Source files
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)

# Target executable
TARGET = todo

# Default target
all: $(TARGET)

# Create directories if they don't exist
$(OBJDIR):
	mkdir -p $(OBJDIR)

$(DATADIR):
	mkdir -p $(DATADIR)

# Build target
$(TARGET): $(OBJDIR) $(DATADIR) $(OBJECTS)
	$(CXX) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

# Compile source files
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# Clean build files
clean:
	rm -rf $(OBJDIR) $(TARGET)

# Clean everything including data
clean-all: clean
	rm -rf $(DATADIR)

# Install (copy to /usr/local/bin)
install: $(TARGET)
	sudo cp $(TARGET) /usr/local/bin/

# Uninstall
uninstall:
	sudo rm -f /usr/local/bin/$(TARGET)

# Run the application
run: $(TARGET)
	./$(TARGET)

# Debug build
debug: CXXFLAGS += -g -DDEBUG
debug: clean $(TARGET)

# Help
help:
	@echo "Available targets:"
	@echo "  all       - Build the application (default)"
	@echo "  clean     - Remove build files"
	@echo "  clean-all - Remove build files and data"
	@echo "  install   - Install to /usr/local/bin"
	@echo "  uninstall - Remove from /usr/local/bin"
	@echo "  run       - Build and run the application"
	@echo "  debug     - Build with debug information"
	@echo "  help      - Show this help message"

.PHONY: all clean clean-all install uninstall run debug help
