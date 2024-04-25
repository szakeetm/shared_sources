################# CMake Project ############################
# The main options of project                              #
############################################################

#This is a separate project, excluded if NO_INTERFACE cmake option is set
#Called from src_shared/CMakeLists.txt

set(PROJECT_NAME src_shared_gui)
project(${PROJECT_NAME} CXX)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${INTERFACE_FILES}
        ${GLAPP_FILES}
         )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        ${HEADER_DIR_5}
        ${HEADER_DIR_7}
        ${IMGUI_DIR}
        ${HEADER_DIR_EXTERNAL}
        )

    find_package(SDL2 CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME}
        PRIVATE
        $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
    )

if(MSVC)
    find_package(OpenGL REQUIRED)
    # 1. link against external libs
    target_link_libraries(${PROJECT_NAME} PUBLIC
            bzip2.lib
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
    find_package(PNG REQUIRED)

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${OPENGL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC ${PNG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)
    target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})

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

#External libraries

target_link_libraries(${PROJECT_NAME} PUBLIC sdl_savepng nativefiledialog-extended-molflow-wrapped)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal::cereal)
target_link_libraries(${PROJECT_NAME} PUBLIC ziplib)
find_package(imgui CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui::imgui)
find_package(implot CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC implot::implot)

#Suppress warnings for external libraries
if(MSVC)
    set(SUPPRESS_WARNINGS_FLAG "/w")
else()
    set(SUPPRESS_WARNINGS_FLAG "-w")
endif()

target_compile_options(sdl_savepng PRIVATE ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(ziplib PRIVATE ${SUPPRESS_WARNINGS_FLAG})

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################


target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)
include(${CMAKE_HOME_DIRECTORY}/src_shared/SetOpenMP.cmake)  #Help macOS find OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "Detected OpenMP version: ${OpenMP_CXX_VERSION}")
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()