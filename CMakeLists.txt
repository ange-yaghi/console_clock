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
    src/executable_listener.cpp
    src/file_manager.cpp
    src/main.cpp
    src/settings.cpp

    # Include files
    include/executable_listener.h
    include/file_manager.h
    include/settings.h
)

# Run helper
add_executable(ConsoleClockRun
    # Source files
    src/run.cpp
)

target_include_directories(ConsoleClock PUBLIC "${PROJECT_BINARY_DIR}")

# Packaging
add_custom_target(console_clock_copy_resources ALL
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/workspace
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/workspace/builds
                    COMMAND ${CMAKE_COMMAND} -E rm -r -f ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock/bin
                    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock/resources
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                            ${CMAKE_SOURCE_DIR}/resources
                            ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock/resources
                    COMMENT "Packaging application folder")

add_custom_target(console_clock_copy_exe ALL
                    COMMAND ${CMAKE_COMMAND} -E copy
                            $<TARGET_FILE:ConsoleClock>
                            ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock/bin
                    COMMAND ${CMAKE_COMMAND} -E copy
                            $<TARGET_FILE:ConsoleClockRun>
                            ${CMAKE_SOURCE_DIR}/workspace/builds/console_clock/bin
                    COMMENT "Copying application executable")

add_dependencies(console_clock_copy_exe ConsoleClock console_clock_copy_resources)
