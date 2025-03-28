cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(crist-project)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
    include
    /usr/local/include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui
)

# Define the source files
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

# ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp
)

# rlImGui source files
set(RLIMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui/rlImGui.cpp
)

# Combine all sources
set(SOURCES ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

# Add the executable
add_executable(crist-project ${SOURCES})

# Optional: Use pkg-config for automatic Raylib detection
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
include_directories(${RAYLIB_INCLUDE_DIRS})

# Link libraries
target_link_libraries(crist-project 
    ${RAYLIB_LIBRARIES}
    GL
    m
    pthread
    dl
    rt
    X11
)

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

