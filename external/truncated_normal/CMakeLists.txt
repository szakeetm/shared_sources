cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME truncated_normal)

# Folders files
set(CPP_DIR_TN src)
set(HEADER_DIR_TN include)

project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

# Source Files
file(GLOB SRC_FILES
		${CPP_DIR_TN}/truncated_normal.cpp
    )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_TN}
        )