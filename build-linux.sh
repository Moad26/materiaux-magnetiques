#!/bin/bash

build_linux() {
    echo "Building for Linux..."
    mkdir -p build-linux
    cd build-linux
    cmake ..
    make -j$(nproc)
    cd ..
    
    # Create distribution directory
    mkdir -p dist/linux
    cp build-linux/crist-project dist/linux/
    cp -r assets dist/linux/
    
    echo "Linux build completed. Output in dist/linux/"
}
