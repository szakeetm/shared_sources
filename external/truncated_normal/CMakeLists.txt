cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME truncated_normal)

# Folders files
set(CPP_DIR_1 src)
set(HEADER_DIR_1 include)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################
if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++17")
else()
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
endif()

################ Files ################
#   --   Add files to project.   --   #
#######################################

# Source Files
file(GLOB SRC_FILES
		${CPP_DIR_1}/truncated_normal.cpp
    )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )
		
add_library(${PROJECT_NAME}_src INTERFACE)
target_include_directories(${PROJECT_NAME}_src INTERFACE include/)