cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME clipper)

# Folders files
set(CPP_DIR_1 ../../src_shared/Clipper)
set(HEADER_DIR_1 ../../src_shared/Clipper)

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

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /MDd /Od /EHsc)
        #for .pdb debugging files
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /Oi /Gy /EHsc)
    endif()
endif()