# CMake structure:
# Separated GUI/CLI/common parts, also separated MOLFLOW/SYNRAD/shared parts
#
# src_shared_core: GUI/CLI, Molflow/Synrad
# src_shared_gui: GUI, Molflow/Synrad -> depends on src_shared_core

cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

#Set variables common for CLI and GUI
project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)

add_subdirectory(compress)
add_subdirectory(external EXCLUDE_FROM_ALL)

add_subdirectory(cmake_modules/src_shared_core) #CLI project, build as library

if(NOT NO_INTERFACE)
        add_subdirectory(cmake_modules/src_shared_gui) #GUI project, build as library
        add_dependencies(src_shared_gui src_shared_core)
endif()