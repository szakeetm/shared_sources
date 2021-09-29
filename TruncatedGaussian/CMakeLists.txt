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
set(CPP_DIR_1 ../../src_shared/TruncatedGaussian)
set(HEADER_DIR_1 ../../src_shared/TruncatedGaussian)

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

if(MSVC)
    set(LINK_DIR_1 "${CMAKE_SOURCE_DIR}/lib_external/win/${MY_BUILD_TYPE}")
    message(${LINK_DIR_1})

    target_include_directories(${PROJECT_NAME} PUBLIC
            #"../../include/windows_only"
            "${CMAKE_CURRENT_SOURCE_DIR}/../../include/windows_only"
            )
    target_link_libraries(${PROJECT_NAME}
            ${LINK_DIR_1}/libgsl.lib
            ${LINK_DIR_1}/libgslcblas.lib
            )
endif(MSVC)
if(NOT MSVC)
    find_package(GSL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GSL_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} ${GSL_LIBRARIES})
endif(NOT MSVC)

################# Flags ################
# Defines Flags for Windows and Linux. #
########################################

# Preprocessor definitions
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
endif()