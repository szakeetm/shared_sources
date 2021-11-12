cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME pugixml)

# Folders files
set(CPP_DIR_1 .)
set(HEADER_DIR_1 .)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

################ Files ################
#   --   Add files to project.   --   #
#######################################

set(SRC_FILES ${CPP_DIR_1}/pugixml.cpp
        ${HEADER_DIR_1}/pugixml.hpp
        ${HEADER_DIR_1}/pugiconfig.hpp
        )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)