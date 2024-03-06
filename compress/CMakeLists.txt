cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME compress)

# Folders files
set(CPP_DIR_1 ../../src_shared/compress)
set(CPP_DIR_2 ../../src_shared)
set(HEADER_DIR_1 ../../src_shared)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

# Definition of Macros
add_definitions(
        -D_MBCS
        -D_CRT_SECURE_NO_WARNINGS
)

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################


################ Files ################
#   --   Add files to project.   --   #
#######################################

set(SRC_FILES compress.cpp
        ${CPP_DIR_2}/File.cpp
        )


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}) #to build executable in main folder

# Add executable to build.
add_executable(${PROJECT_NAME}
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )

target_link_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )

find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(NOT MSVC)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        # add nothing for filesystem
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()
endif(NOT MSVC)