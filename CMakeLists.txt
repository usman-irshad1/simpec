cmake_minimum_required(VERSION 3.16)
project(AdaptiveTrafficSimulator VERSION 2.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Optimization flags
if(MSVC)
    add_compile_options(/W4 /O2)
else()
    add_compile_options(-Wall -Wextra -O2)
endif()

# Include directories
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core
    ${CMAKE_CURRENT_SOURCE_DIR}/src/simulation
    ${CMAKE_CURRENT_SOURCE_DIR}/src/visualization
)

# -----------------------------------------------------------------------------
# Raylib Dependency Discovery
# -----------------------------------------------------------------------------
if(WIN32)
    # Check for local raylib.dll and import library in workspace root
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/raylib.dll")
        add_library(raylib_local SHARED IMPORTED)
        set_target_properties(raylib_local PROPERTIES
            IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/raylib.dll"
            IMPORTED_IMPLIB "${CMAKE_CURRENT_SOURCE_DIR}/raylib.dll"
        )
        set(RAYLIB_LIBRARIES raylib_local opengl32 gdi32 winmm)
    else()
        find_package(raylib 4.0 QUIET)
        if(NOT raylib_FOUND)
            message(STATUS "raylib not found via CMake package, searching system libraries...")
            find_library(RAYLIB_LIB NAMES raylib)
            set(RAYLIB_LIBRARIES ${RAYLIB_LIB} opengl32 gdi32 winmm)
        else()
            set(RAYLIB_LIBRARIES raylib)
        endif()
    endif()
elseif(APPLE)
    find_package(raylib 4.0 QUIET)
    if(NOT raylib_FOUND)
        find_library(RAYLIB_LIB NAMES raylib PATHS /opt/homebrew/lib /usr/local/lib)
        find_library(OPENGL_LIB OpenGL)
        find_library(COCOA_LIB Cocoa)
        find_library(IOKIT_LIB IOKit)
        find_library(COREVIDEO_LIB CoreVideo)
        set(RAYLIB_LIBRARIES ${RAYLIB_LIB} ${OPENGL_LIB} ${COCOA_LIB} ${IOKIT_LIB} ${COREVIDEO_LIB})
    else()
        set(RAYLIB_LIBRARIES raylib)
    endif()
else() # Linux / Unix
    find_package(raylib 4.0 QUIET)
    if(NOT raylib_FOUND)
        find_library(RAYLIB_LIB NAMES raylib)
        set(RAYLIB_LIBRARIES ${RAYLIB_LIB} GL m pthread dl rt X11)
    else()
        set(RAYLIB_LIBRARIES raylib)
    endif()
endif()

# -----------------------------------------------------------------------------
# Executable Targets
# -----------------------------------------------------------------------------

# 1. Main 3D/2D Visual Traffic Simulator
add_executable(Graphics src/visualization/Graphics.cpp)
target_link_libraries(Graphics PRIVATE ${RAYLIB_LIBRARIES})

# Copy raylib.dll to build directory on Windows if present
if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/raylib.dll")
    add_custom_command(TARGET Graphics POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_CURRENT_SOURCE_DIR}/raylib.dll"
        "$<TARGET_FILE_DIR:Graphics>"
    )
endif()

# 2. Headless Simulation & Routing Test Suite
add_executable(test_simulation src/tests/test_simulation.cpp)

# -----------------------------------------------------------------------------
# Install Target
# -----------------------------------------------------------------------------
install(TARGETS Graphics test_simulation DESTINATION bin)
