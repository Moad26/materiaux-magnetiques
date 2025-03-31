#!/bin/bash

# Build script for both Linux and Windows

# Source the build scripts
source build-linux.sh
source build-windows.sh

# Process command-line arguments
if [ "$1" == "linux" ]; then
    build_linux
elif [ "$1" == "windows" ]; then
    build_windows
elif [ "$1" == "all" ]; then
    build_linux
    build_windows
else
    echo "Usage: ./build.sh [linux|windows|all]"
    echo "  linux   - Build for Linux only"
    echo "  windows - Build for Windows only"
    echo "  all     - Build for both platforms"
fi
