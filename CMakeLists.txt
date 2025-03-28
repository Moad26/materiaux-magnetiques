cmake_minimum_required(VERSION 3.10)

project(crist-project)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(include /usr/local/include)

# Define the source files
file(GLOB_RECURSE SOURCES "src/*.cpp")

# Add the executable
add_executable(crist-project ${SOURCES})

# Link the Raylib shared library (system-wide installation)
target_link_libraries(crist-project /usr/local/lib/libraylib.so)

# Link system libraries (GL, m, pthread, etc.)
target_link_libraries(
  crist-project
  GL
  m
  pthread
  dl
  rt
  X11)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Optional: Use pkg-config for automatic Raylib detection
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
include_directories(${RAYLIB_INCLUDE_DIRS})
target_link_libraries(crist-project ${RAYLIB_LIBRARIES})
