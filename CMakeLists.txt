cmake_minimum_required(VERSION 3.10)
project(dn4l_cpp_prototype LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the Turbo Vision library as a subdirectory
# Assumes magiblot's tvision is in a 'tvision' subdirectory
add_subdirectory(tvision)

# Define the dn4l executable
add_executable(dn4l
    dn4l.cpp
    dnapp.cpp
    dblwnd.cpp
    flpanel.cpp
    dnlogger.cpp
)

# Link dn4l against Turbo Vision
# The target 'tvision' is defined in tvision/CMakeLists.txt
target_link_libraries(dn4l PRIVATE tvision)

# Set include directories for dn4l itself (if headers are in a different dir)
target_include_directories(dn4l PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

# Platform-specific settings (ncurses is common for TV on Unix)
if(UNIX AND NOT APPLE)
    find_package(Curses REQUIRED)
    if(CURSES_FOUND)
        target_link_libraries(dn4l PRIVATE ${CURSES_LIBRARY})
    endif()
endif()

# Optional: Set output directory for the executable
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR})