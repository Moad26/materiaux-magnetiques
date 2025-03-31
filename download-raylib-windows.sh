#!/bin/bash

# Download and set up raylib for Windows cross-compilation
RAYLIB_VERSION="4.0.0"  # Changed from 4.5.0 to 4.0.0
RAYLIB_URL="https://github.com/raysan5/raylib/releases/download/${RAYLIB_VERSION}/raylib-${RAYLIB_VERSION}_win64_mingw-w64.zip"

echo "Downloading raylib ${RAYLIB_VERSION} for Windows..."
mkdir -p temp
cd temp
wget -q "${RAYLIB_URL}" -O raylib.zip
unzip -q raylib.zip

cd ..
mkdir -p raylib/include raylib/lib

# Copy headers and libraries
cp -r temp/raylib-${RAYLIB_VERSION}_win64_mingw-w64/include/* raylib/include/
cp -r temp/raylib-${RAYLIB_VERSION}_win64_mingw-w64/lib/* raylib/lib/

# Clean up
rm -rf temp

echo "Raylib for Windows setup complete."
