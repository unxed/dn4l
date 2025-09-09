# Require a version of CMake that supports C++20 features.
cmake_minimum_required(VERSION 3.16)
project(dn4l LANGUAGES CXX)

# Set the C++ standard to C++20 for modern features like std::format, std::ranges, etc.
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF) # A best practice for portability.

# Add the tvision subdirectory, which contains the UI framework.
add_subdirectory(tvision)

# Define the main executable and its source files.
add_executable(dn4l
    dn4l.cpp
    dnapp.cpp
    dblwnd.cpp
    flpanel.cpp
    dnlogger.cpp
)

# Link the executable against the tvision library.
# The 'PRIVATE' keyword ensures that this dependency is not propagated to other targets
# that might link against dn4l (if it were a library).
target_link_libraries(dn4l PRIVATE tvision)

# Explicitly state that this target requires C++20 compiler features.
# This is a more robust way to ensure standard compliance than just setting the variable.
target_compile_features(dn4l PRIVATE cxx_std_20)

# Platform-specific settings for linking against ncurses on Unix-like systems.
if(UNIX AND NOT APPLE)
    find_package(Curses REQUIRED)
    if(CURSES_FOUND)
        target_link_libraries(dn4l PRIVATE ${CURSES_LIBRARY})
    endif()
endif()

# Set the output directory for the final executable.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})
