cmake_minimum_required(VERSION 3.5)
project(bzip2)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CYGWIN)
    set(CMAKE_CXX_EXTENSIONS OFF)
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

