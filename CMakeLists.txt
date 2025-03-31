cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(crist-project)

# Set the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Define platform-specific settings
if(WIN32)
    # Windows-specific settings
    set(PLATFORM_LIBS winmm gdi32)
    
    # Copy assets to build directory for Windows
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/assets $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets)
else()
    # Linux-specific settings
    set(PLATFORM_LIBS GL m pthread dl rt X11)
endif()

# Include directories
include_directories(
    include
    ${CMAKE_CURRENT_SOURCE_DIR}/imgui
    ${CMAKE_CURRENT_SOURCE_DIR}/rlImGui
)

# Add Raylib include path based on platform
if(WIN32)
    # For Windows cross-compilation, explicitly set Raylib paths
    include_directories(${CMAKE_SOURCE_DIR}/raylib/include)
    link_directories(${CMAKE_SOURCE_DIR}/raylib/lib)
else()
    # Use pkg-config for Linux
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(RAYLIB REQUIRED raylib)
    include_directories(${RAYLIB_INCLUDE_DIRS})
endif()

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

# Link libraries based on platform
if(WIN32)
    target_link_libraries(crist-project raylib ${PLATFORM_LIBS})
else()
    target_link_libraries(crist-project ${RAYLIB_LIBRARIES} ${PLATFORM_LIBS})
endif()

# Set the output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})

# Add Windows-specific executable properties
if(WIN32)
    set_target_properties(crist-project PROPERTIES SUFFIX ".exe")
endif()
