cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME fmtlib)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} C CXX)

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

# Source Files

# Download and unpack fmt at configure time
configure_file(CMakeLists.txt.in fmt-download/CMakeLists.txt)
execute_process(COMMAND ${CMAKE_COMMAND} -G "${CMAKE_GENERATOR}" .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fmt-download )
if(result)
    message(FATAL_ERROR "CMake step for nfd failed: ${result}")
endif()
execute_process(COMMAND ${CMAKE_COMMAND} --build .
        RESULT_VARIABLE result
        WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/fmt-download )
if(result)
    message(FATAL_ERROR "Build step for fmt failed: ${result}")
endif()

# Add fmt directly to our build. This defines
# the fmt target.
add_subdirectory(${CMAKE_CURRENT_BINARY_DIR}/fmt-src
        ${CMAKE_CURRENT_BINARY_DIR}/fmt-build
        EXCLUDE_FROM_ALL
        )

# The gtest/gtest_main targets carry header search path
# dependencies automatically when using CMake 2.8.11 or
# later. Otherwise we have to add them here ourselves.
#[[if (CMAKE_VERSION VERSION_LESS 2.8.11)
    include_directories("${fmtlib_BINARY_DIR}/fmt-src/include")
endif()]]
include_directories(SYSTEM ${CMAKE_CURRENT_BINARY_DIR}/fmt-src/include)

# Get the current working branch
execute_process(
        COMMAND git rev-parse --abbrev-ref HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_BRANCH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

# Get the latest commit hash
execute_process(
        COMMAND git rev-parse HEAD
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE GIT_COMMIT_HASH
        OUTPUT_STRIP_TRAILING_WHITESPACE)

#add_subdirectory(nativefiledialog-extended)

#add_library(${PROJECT_NAME} STATIC ${WRAPPER_SRC})

add_library(${PROJECT_NAME}_src INTERFACE)
target_include_directories(${PROJECT_NAME}_src INTERFACE include/)
target_include_directories(${PROJECT_NAME}_src INTERFACE ${CMAKE_CURRENT_BINARY_DIR}/fmt-src/include/)
