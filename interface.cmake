################# CMake Project ############################
# The main options of project                              #
############################################################

#This is a separate project, excluded if NO_INTERFACE cmake option is set

set(PROJECT_NAME shared_interface)
project(${PROJECT_NAME} CXX)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${INTERFACE_FILES}
        ${GL_FILES}
        )


target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        )

target_include_directories(${PROJECT_NAME} PUBLIC
        ${HEADER_DIR_5}
        ${HEADER_DIR_6}
        ${HEADER_DIR_7}
        ${IMGUI_DIR}
        ${HEADER_DIR_ZIP}
        ${HEADER_DIR_ZIP2}
        )

if(MSVC)
    find_package(OpenGL REQUIRED)
    # 1. link against external libs
    target_link_libraries(${PROJECT_NAME} PUBLIC
            bzip2.lib
            libcurl_a_x64.lib
            SDL2.lib
            SDL2main.lib
            lzma.lib
            opengl32#.lib
            user32.lib
            shell32.lib
            ole32.lib
            )
ELSE() #not MSVC

    if(APPLE)
        #link to self-built sdl shared lib
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit")

        FIND_PACKAGE(SDL2 REQUIRED)
        Message("")
        Message( STATUS "FINDING SDL2" )
        IF (${SDL2_FOUND})
            Message( STATUS "SDL2_FOUND: " ${SDL2_FOUND})
            Message( STATUS "SDL2_INCLUDE_DIR:" ${SDL2_INCLUDE_DIR})
            Message( STATUS "SDL2_LIBRARY: " ${SDL2_LIBRARY})
        ELSE()
            Message( STATUS "SDL2_FOUND: " ${SDL2_FOUND})
            Message( FATAL_ERROR "SDL2 NOT FOUND" )
        ENDIF()

    else()
        # Use the package PkgConfig to detect GTK+ headers/library files
        FIND_PACKAGE(PkgConfig REQUIRED)
        PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

        # Setup CMake to use GTK+, tell the compiler where to look for headers
        # and to the linker where to look for libraries
        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
        LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

        # Add other flags to the compiler
        ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

        target_link_libraries(${PROJECT_NAME} PUBLIC ${GTK3_LIBRARIES})

        find_package(X11 REQUIRED)
        target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${X11_INCLUDE_DIRS})
    endif()

    find_package(OpenGL REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${OPENGL_INCLUDE_DIRS})

    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${SDL2_INCLUDE_DIRS})

    find_package(PNG REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${PNG_INCLUDE_DIRS})

    find_package(GSL REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${GSL_INCLUDE_DIRS})

    find_package(CURL REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${CURL_INCLUDE_DIRS})

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${SDL2_LIBRARY})

    target_link_libraries(${PROJECT_NAME} PUBLIC ${PNG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${GSL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${CURL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})
    #target_link_libraries(${PROJECT_NAME} ${GSLCBLAS_LIBRARIES})

    #for shared memory
    find_library(LIBRT rt)
    if(LIBRT)
        target_link_libraries(${PROJECT_NAME} PUBLIC ${LIBRT})
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${PROJECT_NAME} PUBLIC c++fs)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        #don´t add anything for filesystem
    else()
        target_link_libraries(${PROJECT_NAME} PUBLIC stdc++fs)
    endif()

    #external libraries from our project
    message(Shared CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif() #NOT MSVC

target_link_libraries(${PROJECT_NAME} PUBLIC pugixml clipper sdl_savepng truncatedgaussian nativefiledialog)
target_link_libraries(${PROJECT_NAME} PUBLIC fmtlib_src) # header include
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal)
target_link_libraries(${PROJECT_NAME} PUBLIC ziplib)

target_link_libraries(${PROJECT_NAME}  PUBLIC imgui implot)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)