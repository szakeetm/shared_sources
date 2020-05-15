cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# Project
set(PROJECT_NAME nativefiledialog)

# Folders files
set(CPP_DIR_1 ./molflow_wrapper)
set(CPP_DIR_2 .)
set(HEADER_DIR_1 ./molflow_wrapper)
set(HEADER_DIR_2 .)

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} C CXX)

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

# Source Files

set(WRAPPER_SRC molflow_wrapper/nfd_wrapper.cpp)
set(NFD_SRC nfd_common.c)
# win, cocoa or gtk
if (WIN32)
    #REQUIRE_INCLUDE_FILE(windows.h HAS_WINDOWS)
    list(APPEND NFD_SRC nfd_win.cpp)
elseif (APPLE)
    #REQUIRE_INCLUDE_FILE(AppKit/AppKit.h HAS_APPKIT)
    # use custom compiler (gcc or clang) for the c++ wrapper
    set(CMAKE_C_COMPILER clang)
    list(APPEND NFD_SRC nfd_cocoa.m)
elseif (UNIX)
    #REQUIRE_INCLUDE_FILE(gtk/gtk.h HAS_GTK)
    list(APPEND NFD_SRC nfd_gtk.c)
elseif (UNIX)
    message(FATAL_ERROR "Cannot detect your system, please report to https://github.com/aarcangeli/nativefiledialog-cmake/issues")
endif ()

add_library(${PROJECT_NAME} STATIC ${WRAPPER_SRC})
add_library(${PROJECT_NAME}_src STATIC ${NFD_SRC})
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_src)

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        )
target_link_libraries(${PROJECT_NAME} ${PROJECT_NAME}_src)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

IF (WIN32)

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
ENDIF()

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
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /EHsc)
    endif()
endif()