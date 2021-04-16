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

set(SRC_FILES ${CPP_DIR_1}/compress.cpp
        ${CPP_DIR_2}/File.cpp
        ${HEADER_DIR_1}/File.h
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

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

if(MSVC)

endif(MSVC)
if(NOT MSVC)
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        # add nothing for filesystem
    else()
        target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
    endif()
endif(NOT MSVC)

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