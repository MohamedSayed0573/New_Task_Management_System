# ================================================================
# Modern Makefile for Todo List Manager
# ================================================================

# ===================
# Compiler Settings
# ===================
CXX      = g++
CXXFLAGS = -std=c++20 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS  = 

# ===================
# Build Configuration
# ===================
BUILD_TYPE ?= release

ifeq ($(BUILD_TYPE),debug)
    CXXFLAGS += -g -O0 -DDEBUG -fsanitize=address,undefined
    LDFLAGS  += -fsanitize=address,undefined
    $(info Building in DEBUG mode)
else ifeq ($(BUILD_TYPE),profile)
    CXXFLAGS += -O2 -pg -fno-omit-frame-pointer
    LDFLAGS  += -pg
    $(info Building in PROFILE mode)
else
    CXXFLAGS += -O3 -DNDEBUG -march=native
    LDFLAGS  += -s
    $(info Building in RELEASE mode)
endif

# ===================
# Directories
# ===================
SRCDIR = src
OBJDIR = obj
INCDIR = include

# ===================
# Files
# ===================
SOURCES = $(wildcard $(SRCDIR)/*.cpp)
OBJECTS = $(SOURCES:$(SRCDIR)/%.cpp=$(OBJDIR)/%.o)
TARGET  = todo

# ===================
# Dependencies
# ===================
DEPS = $(OBJECTS:.o=.d)

# ===================
# Build Rules
# ===================

# Default target
all: $(TARGET)

# Create object directory
$(OBJDIR):
	@mkdir -p $(OBJDIR)

# Link executable
$(TARGET): $(OBJECTS)
	@echo "Linking $@..."
	@$(CXX) $(OBJECTS) -o $@ $(LDFLAGS)
	@echo "Build complete: $@"

# Compile source files with dependency generation
$(OBJDIR)/%.o: $(SRCDIR)/%.cpp | $(OBJDIR)
	@echo "Compiling $<..."
	@$(CXX) $(CXXFLAGS) -MMD -MP -c $< -o $@

# Include dependency files
-include $(DEPS)

# ===================
# Utility Targets
# ===================

# Clean build artifacts
clean:
	@echo "Cleaning build files..."
	@rm -rf $(OBJDIR) $(TARGET)

# Clean everything
distclean: clean
	@echo "Cleaning all generated files..."
	@rm -rf data/*.json

# Build and run
run: $(TARGET)
	@echo "Running $(TARGET)..."
	@./$(TARGET)

# Debug build shortcut
debug:
	@$(MAKE) BUILD_TYPE=debug

# Profile build shortcut
profile:
	@$(MAKE) BUILD_TYPE=profile

# Deploy target: clean, debug build, and install
deploy:
	@echo "Starting deployment process..."
	@$(MAKE) clean
	@$(MAKE) BUILD_TYPE=debug
	@$(MAKE) install
	@echo "Deployment complete!"

# Install to system
install: $(TARGET)
	@echo "Installing $(TARGET) to /usr/local/bin..."
	@sudo cp $(TARGET) /usr/local/bin/
	@echo "Installation complete!"

# Uninstall from system
uninstall:
	@echo "Removing $(TARGET) from /usr/local/bin..."
	@sudo rm -f /usr/local/bin/$(TARGET)
	@echo "Uninstall complete!"

# Show help
help:
	@echo "Todo List Manager - Build System"
	@echo "================================="
	@echo ""
	@echo "Targets:"
	@echo "  all       - Build the application (default)"
	@echo "  clean     - Remove build artifacts"
	@echo "  distclean - Remove all generated files"
	@echo "  run       - Build and run the application"
	@echo "  deploy    - Clean, debug build, and install"
	@echo "  install   - Install to system (requires sudo)"
	@echo "  uninstall - Remove from system (requires sudo)"
	@echo "  help      - Show this help message"
	@echo ""
	@echo "Build Types:"
	@echo "  make BUILD_TYPE=release  - Optimized build (default)"
	@echo "  make BUILD_TYPE=debug    - Debug build with sanitizers"
	@echo "  make BUILD_TYPE=profile  - Profiling build"
	@echo ""
	@echo "Examples:"
	@echo "  make                     - Release build"
	@echo "  make debug               - Debug build"
	@echo "  make run                 - Build and run"
	@echo "  make clean all           - Clean and rebuild"
	@echo "  make deploy              - Clean, debug build, and install"

# ===================
# Phony Targets
# ===================
.PHONY: all clean distclean run debug profile deploy install uninstall help

# ===================
# Special Targets
# ===================
.DEFAULT_GOAL := all
.SUFFIXES:
