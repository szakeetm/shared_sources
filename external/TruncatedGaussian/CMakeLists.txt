cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

# Project
set(PROJECT_NAME truncatedgaussian)

############## CMake Project ################
#        The main options of project        #
#############################################

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

set(SRC_FILES ${CPP_DIR_1}/rtnorm.cpp )
set(HEADER_FILES ${HEADER_DIR_1}/rtnorm.hpp)

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})



target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        )


    find_package(GSL REQUIRED)
    target_link_libraries(${PROJECT_NAME} GSL::gsl GSL::gslcblas)
