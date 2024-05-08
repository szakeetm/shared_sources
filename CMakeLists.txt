cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

#Set variables common for CLI and GUI
project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

set(HEADER_DIR_EXTERNAL ${CPP_DIR_SRC_SHARED}/external) #applied in src_shared_gui and src_shared_core
set(INTERFACE_DIR ${CPP_DIR_SRC_SHARED}/Interface) #Also used by cli (camera views saving, etc)

if(APPLE)
include(${CPP_DIR_SRC_SHARED}/cmake_modules/SetOpenMP.cmake)  #Help macOS find OpenMP
endif()
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
        message(STATUS "Detected OpenMP version: ${OpenMP_CXX_VERSION}")
endif()

add_subdirectory(compress)
add_subdirectory(external EXCLUDE_FROM_ALL)

add_subdirectory(cmake_modules/src_shared_core) #CLI project, build as library

if(NOT NO_INTERFACE)
        add_subdirectory(cmake_modules/src_shared_gui) #GUI project, build as library
endif()