cmake_minimum_required(VERSION 2.6)
project(bzip2)

if (CMAKE_VERSION VERSION_LESS "3.1")
    add_definitions(-std=c++11)
else()
    set(CMAKE_CXX_STANDARD 11)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
    if(NOT CYGWIN)
        set(CMAKE_CXX_EXTENSIONS OFF)
    endif()
endif()

set(headers
        bzlib.h
        bzlib_private.h
        )

set(src
        blocksort.c
        bzerror.c
        bzlib.c
        compress.c
        crctable.c
        decompress.c
        huffman.c
        randtable.c
        )

set(sources
        ${headers}
        ${src}
        )

add_library(${PROJECT_NAME} STATIC ${sources})
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_DEFINITIONS "BZ_NO_STDIO")
if(MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS " /wd4996 /wd4244")
endif()

