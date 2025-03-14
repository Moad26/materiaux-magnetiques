cmake_minimum_required(VERSION 3.10)
project(crist-project)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Use pkg-config to find Raylib
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)

include_directories(${RAYLIB_INCLUDE_DIRS})

add_executable(crist-project src/main.cpp)

target_link_libraries(crist-project ${RAYLIB_LIBRARIES})
