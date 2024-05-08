cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME clipper2)

# Folders files
set(HEADER_DIR_CLIPPER2 include/clipper2)
set(CPP_DIR_CLIPPER2 src)

############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)
set(CMAKE_CXX_STANDARD 17)


################ Files ################
#   --   Add files to project.   --   #
#######################################

# Source Files
file(GLOB SRC_FILES
		${CPP_DIR_CLIPPER2}/clipper.engine.cpp
		${CPP_DIR_CLIPPER2}/clipper.offset.cpp
		${CPP_DIR_CLIPPER2}/clipper.rectclip.cpp
    )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_CLIPPER2}
        )