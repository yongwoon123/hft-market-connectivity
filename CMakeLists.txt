cmake_minimum_required(VERSION 3.20)
project(hft-engine CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Compiler flags
add_compile_options(-Wall -Wextra -Wpedantic)

# Subdirectories
add_subdirectory(src)
enable_testing()
include(GoogleTest)

add_subdirectory(tests)
add_subdirectory(tools)