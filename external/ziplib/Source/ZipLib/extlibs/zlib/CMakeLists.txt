cmake_minimum_required(VERSION 3.5)
project(zlib)


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if(NOT CYGWIN)
    set(CMAKE_CXX_EXTENSIONS OFF)
endif()

set(headers
        crc32.h
        deflate.h
        gzguts.h
        inffast.h
        inffixed.h
        inflate.h
        inftrees.h
        trees.h
        zconf.h
        zlib.h
        zutil.h
        )

set(src
        adler32.c
        compress.c
        crc32.c
        deflate.c
        infback.c
        inffast.c
        inflate.c
        inftrees.c
        trees.c
        uncompr.c
        zutil.c
        )

set(sources
        ${headers}
        ${src}
        )

add_library(${PROJECT_NAME} STATIC ${sources})
if(MSVC)
set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS " /wd4996")
endif()