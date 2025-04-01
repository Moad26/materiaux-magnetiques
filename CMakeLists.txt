cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(crist-project)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Include directories
include_directories(
  include
  ${CMAKE_CURRENT_SOURCE_DIR}/imgui
  ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui
  ${CMAKE_CURRENT_SOURCE_DIR}/raylib/include
)

# Link directories for raylib
link_directories(${CMAKE_CURRENT_SOURCE_DIR}/raylib/lib)

# Define the source files
file(GLOB_RECURSE PROJECT_SOURCES "src/*.cpp")

# ImGui source files
set(IMGUI_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_demo.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_draw.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_tables.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui/imgui_widgets.cpp)

# rlImGui source files
set(RLIMGUI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui/rlImGui.cpp)

# Combine all sources
set(SOURCES ${PROJECT_SOURCES} ${IMGUI_SOURCES} ${RLIMGUI_SOURCES})

# Add the executable
add_executable(crist-project ${SOURCES})

# For Windows cross-compilation
if(MINGW)
    message(STATUS "Cross-compiling for Windows")
    
    # Windows specific libraries
    target_link_libraries(crist-project
        raylib
        winmm
        gdi32
        opengl32
        -static-libgcc
        -static-libstdc++
    )
    
    # Set output directory for Windows binaries
    set_target_properties(crist-project PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/dist/windows
    )
    
    # Copy assets to Windows dist directory
    add_custom_command(TARGET crist-project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets ${CMAKE_SOURCE_DIR}/dist/windows/assets
    )
    
    # Copy required DLLs
    add_custom_command(TARGET crist-project POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
        ${CMAKE_SOURCE_DIR}/raylib/lib/raylib.dll ${CMAKE_SOURCE_DIR}/dist/windows/
    )
else()
    # For Linux/UNIX build
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
    include_directories(${RAYLIB_INCLUDE_DIRS})
    
    target_link_libraries(crist-project
        ${RAYLIB_LIBRARIES}
        GL
        m
        pthread
        dl
        rt
        X11
    )
endif()
