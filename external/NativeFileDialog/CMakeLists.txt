cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME nativefiledialog-extended-molflow-wrapped)

# Folders files
set(WRAPPER_DIR molflow_wrapper)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} C CXX)

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

# Source Files

set(WRAPPER_SRC ${WRAPPER_DIR}/nfd_wrapper.cpp)

# Add nfd directly to our build. This defines
# the nfd target.
set(NFD_EXT_DIR nativefiledialog-extended)
set(NFD_INCLUDE_DIR ${NFD_EXT_DIR}/src/include)
add_subdirectory(${NFD_EXT_DIR})

add_library(${PROJECT_NAME} STATIC ${WRAPPER_SRC})

target_include_directories(${PROJECT_NAME} PRIVATE ${WRAPPER_DIR} ${NFD_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PRIVATE nfd) #nativefiledialog-extended's default target name is 'nfd'

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

#[[IF (WIN32)

ELSEIF(APPLE)

    target_link_libraries(${PROJECT_NAME}_src
            "-framework AppKit"
            )
ELSE()
    # Use the package PkgConfig to detect GTK+ headers/library files
    FIND_PACKAGE(PkgConfig REQUIRED)
    PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

    # Setup CMake to use GTK+, tell the compiler where to look for headers
    # and to the linker where to look for libraries
    INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
    LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

    # Add other flags to the compiler
    ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})
    target_link_libraries(${PROJECT_NAME}_src ${GTK3_LIBRARIES})
ENDIF()]]
