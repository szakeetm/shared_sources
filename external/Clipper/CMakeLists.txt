cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME clipper)

# Folders files
set(CPP_DIR_1 .)
set(HEADER_DIR_1 .)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################


################ Files ################
#   --   Add files to project.   --   #
#######################################

# Source Files
file(GLOB SRC_FILES
        ${CPP_DIR_1}/clipper.cpp
        ${HEADER_DIR_1}/clipper.hpp
        )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )
