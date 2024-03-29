cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

set(PROJECT_NAME compress)

# Folders files
set(CPP_DIR_1 ../../src_shared/compress)
set(CPP_DIR_2 ../../src_shared)
set(HEADER_DIR_1 ../../src_shared)
set(HEADER_DIR_2 ../../src_shared/external)

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
        )


set(EXECUTABLE_OUTPUT_PATH ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}) #to build executable in main folder

# Add executable to build.
add_executable(${PROJECT_NAME}
        ${SRC_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        )

target_link_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        )

target_link_libraries(${PROJECT_NAME} PUBLIC fmtlib_src) # header include
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)