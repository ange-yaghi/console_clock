cmake_minimum_required(VERSION 3.10)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Project Name
project(ConsoleClock VERSION 1.0)

# C Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

configure_file(config/console_clock_config.h.in console_clock.h)

# Executable
add_executable(ConsoleClock
    # Source files
    src/main.cpp
    src/file_manager.cpp

    # Include files
    include/file_manager.h
)

target_include_directories(ConsoleClock PUBLIC "${PROJECT_BINARY_DIR}")
