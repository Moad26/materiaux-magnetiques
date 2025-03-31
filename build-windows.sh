#!/bin/bash

build_windows() {
    echo "Building for Windows..."
    
    # Check if MinGW is installed
    if ! command -v x86_64-w64-mingw32-gcc &> /dev/null; then
        echo "MinGW not found. Installing cross-compile tools..."
        sudo apt update
        sudo apt install -y mingw-w64
    fi
    
    # Create build directory
    mkdir -p build-windows
    cd build-windows
    
    # Configure with CMake for Windows
    cmake .. -DCMAKE_TOOLCHAIN_FILE=../windows-toolchain.cmake
    
    # Build
    make -j$(nproc)
    cd ..
    
    # Create distribution directory
    mkdir -p dist/windows
    cp build-windows/crist-project.exe dist/windows/
    cp -r assets dist/windows/
    
    # Copy required DLLs
    if [ -d "raylib/lib" ]; then
        cp raylib/lib/*.dll dist/windows/
    else
        echo "Warning: raylib DLLs not found. Please copy them manually to dist/windows/"
    fi
    
    echo "Windows build completed. Output in dist/windows/"
}
