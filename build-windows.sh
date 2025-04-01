#!/bin/bash

# Exit on error
set -e

# Create necessary directories
mkdir -p build-windows
mkdir -p dist/windows

echo "Building Windows version using MinGW cross-compiler..."

# Check if raylib is downloaded
if [ ! -d "raylib" ] || [ ! -d "raylib/lib" ]; then
    echo "Setting up raylib for Windows..."
    mkdir -p raylib
    cd raylib
    
    # Download Windows raylib 5.5
    wget -nc https://github.com/raysan5/raylib/releases/download/5.5/raylib-5.5_win64_mingw-w64.zip
    
    # Extract
    unzip -o raylib-5.5_win64_mingw-w64.zip
    
    # Copy the files
    cp -rf raylib-5.5_win64_mingw-w64/* .
    
    # Clean up
    rm raylib-5.5_win64_mingw-w64.zip
    
    cd ..
fi

# Navigate to build directory
cd build-windows

# Run CMake with the MinGW toolchain
cmake .. -DCMAKE_TOOLCHAIN_FILE=../mingw-toolchain.cmake

# Build the project
make -j$(nproc)

echo "Build completed! Windows executable is in dist/windows folder."
cd ..

# Create a ZIP archive of the Windows directory
echo "Creating ZIP archive of Windows build..."
zip -r dist/windows.zip dist/windows

echo "ZIP archive created: dist/windows.zip"
