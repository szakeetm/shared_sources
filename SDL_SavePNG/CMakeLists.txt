cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

################### Variables. ####################
# Change if you want modify path or other values. #
###################################################

# Project
set(PROJECT_NAME sdl_savepng)

# Folders files
set(CPP_DIR_1 ../../src_shared/SDL_SavePNG)
set(HEADER_DIR_1 ../../src_shared/SDL_SavePNG)
IF (WIN32)
    set(HEADER_DIR_2 ../../include/windows_only/png)
    set(HEADER_DIR_3 ../../include/windows_only)
ENDIF()
############## CMake Project ################
#        The main options of project        #
#############################################

project(${PROJECT_NAME} CXX)

################ Files ################
#   --   Add files to project.   --   #
#######################################

set(SRC_FILES ${CPP_DIR_1}/savepng.cpp
        ${HEADER_DIR_1}/savepng.h
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})

# Source Files
IF (WIN32)
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            ${HEADER_DIR_2}
            ${HEADER_DIR_3}
            )
ELSEIF(APPLE)
    # apple linking against a shipped version of SDL2
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            )

    #find_package(SDL2 REQUIRED)
    #target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
    #set(LINK_DIR_2 ../../lib_external/mac/SDL2)
    #get_filename_component(ABS_LINK_DIR_2 "${LINK_DIR_2}" REALPATH)
    #add_library( libSDL2 STATIC IMPORTED GLOBAL)
    #set_target_properties( libSDL2 PROPERTIES IMPORTED_LOCATION ${ABS_LINK_DIR_2}/libSDL2.a )
    #target_link_libraries(${PROJECT_NAME} libSDL2) # from ./lib/
    #message(${ABS_LINK_DIR_2}/libSDL2.a)

    find_package(PNG REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIRS})
    target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})
ELSE()
    target_include_directories(${PROJECT_NAME} PRIVATE
            ${HEADER_DIR_1}
            )

    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

    find_package(PNG REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})
ENDIF()

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

# Preprocessor definitions
if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /MDd /Od /EHsc)
        set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /Zi")
        set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "${CMAKE_SHARED_LINKER_FLAGS_DEBUG} /DEBUG:FULL /OPT:REF /OPT:ICF")
        #set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS_DEBUG "-DEBUG:FULL")
    endif()
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Release")
    target_compile_definitions(${PROJECT_NAME} PRIVATE)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE  /W3 /GL /Oi /Gy /EHsc)
    endif()
endif()