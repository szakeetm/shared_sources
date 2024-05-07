cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

# Project
set(PROJECT_NAME sdl_savepng)

# Folders files
set(CPP_DIR_1 .)
set(HEADER_DIR_1 .)
IF (WIN32)
    set(HEADER_DIR_2 ${CMAKE_HOME_DIRECTORY}/include/windows_only/png)
    set(HEADER_DIR_3 ${CMAKE_HOME_DIRECTORY}/include/windows_only)
ENDIF()
############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

################ Files ################
#   --   Add files to project.   --   #
#######################################


set(SRC_FILES
        ${CPP_DIR_1}/savepng.cpp
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

find_package(PNG REQUIRED)
target_include_directories(${PROJECT_NAME} SYSTEM PRIVATE ${PNG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})

# Source Files
IF (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            ${HEADER_DIR_2}
            ${HEADER_DIR_3}
            )

ELSE()
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            )


ENDIF()