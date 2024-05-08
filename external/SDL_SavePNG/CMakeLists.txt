cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

# Project
set(PROJECT_NAME sdl_savepng)

# Folders files
set(CPP_DIR_SAVEPNG .)
set(HEADER_DIR_SAVEPNG .)

project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

set(SRC_FILES_SAVEPNG
        ${CPP_DIR_SAVEPNG}/savepng.cpp
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES_SAVEPNG})

find_package(PNG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE PNG::PNG)

# Source Files
target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_SAVEPNG}

        )