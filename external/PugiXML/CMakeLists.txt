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

# Preprocessor definitions
#[[
if(CMAKE_BUILD_TYPE MATCHES Debug|RelWithDebInfo)
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
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /EHsc)
    endif()
endif()]]
