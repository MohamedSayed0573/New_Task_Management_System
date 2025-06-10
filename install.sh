#!/bin/bash

# Todo Application Installer
# Installs the todo application system-wide

set -e

echo "📋 Todo Application Installer"
echo "=============================="

# Check if running as root for system-wide install
if [ "$EUID" -eq 0 ]; then
    INSTALL_DIR="/usr/local/bin"
    echo "Installing system-wide to $INSTALL_DIR"
else
    INSTALL_DIR="$HOME/.local/bin"
    echo "Installing for current user to $INSTALL_DIR"
    mkdir -p "$INSTALL_DIR"
fi

# Build the application
echo "🔨 Building application..."
make clean
make

# Install the binary
echo "📦 Installing todo to $INSTALL_DIR..."
cp todo "$INSTALL_DIR/"
chmod +x "$INSTALL_DIR/todo"

# Check if directory is in PATH
if [[ ":$PATH:" != *":$INSTALL_DIR:"* ]]; then
    echo ""
    echo "⚠️  WARNING: $INSTALL_DIR is not in your PATH"
    echo "Add this line to your ~/.bashrc or ~/.zshrc:"
    echo "export PATH=\"$INSTALL_DIR:\$PATH\""
fi

echo ""
echo "✅ Installation complete!"
echo "🚀 You can now run: todo --help"
