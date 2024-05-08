cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

set(PROJECT_NAME compress)

project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

# Definition of Macros
add_definitions(
        -D_MBCS
        -D_CRT_SECURE_NO_WARNINGS
)

set(SRC_FILES_COMPRESS
        compress.cpp
        ${CPP_DIR_SRC_SHARED}/File.cpp
)

set(EXECUTABLE_OUTPUT_PATH ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}) #to build executable in main folder

# Add executable to build.
add_executable(${PROJECT_NAME}
        ${SRC_FILES_COMPRESS}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_SRC_SHARED}
        )

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)