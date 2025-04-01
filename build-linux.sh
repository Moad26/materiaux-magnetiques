#!/bin/bash

build_linux() {
    echo "Building for Linux..."
    mkdir -p build
    cd build
    cmake ..
    make -j$(nproc)
    cd ..
# Create distribution directory
    mkdir -p dist/linux
    cp build/crist-project dist/linux/
    cp -r assets dist/linux/
    echo "Linux build completed. Output in dist/linux/"
    # Create a ZIP archive of the Linux build
    echo "Creating ZIP archive of Linux build..."
    zip -r dist/linux.zip dist/linux
    echo "ZIP archive created: dist/linux.zip"
}

build_linux
