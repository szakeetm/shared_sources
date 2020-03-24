cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# First process subdirectories related to external projects
add_subdirectory(compress)
add_subdirectory(SDL_SavePNG)
add_subdirectory(Clipper)
add_subdirectory(PugiXML)
add_subdirectory(NativeFileDialog)
add_subdirectory(TruncatedGaussian)

# Project
#get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
#set(DEPENDENCIES_DIR ${PROJECT_DIR}/dependencies)

set(PROJECT_NAME simulator_shared)

# Folders files
set(HEADER_DIR_1 .)
set(HEADER_DIR_2 ./GLApp)
set(HEADER_DIR_3 ./GLApp/GLChart)
set(HEADER_DIR_4 ../include)
set(COPY_DIR ../copy_to_build/)

IF (WIN32)
    # set stuff for windows

    set(HEADER_DIR_5 ../include/windows_only)
    set(HEADER_DIR_6 ../include/windows_only/png)

    set(LINK_DIR_1 ../lib/win/${MY_BUILD_TYPE})
    #set(LINK_DIR_2 ../../_molflow/molflow_project/lib/lib)
    #set(LINK_DIR_3 ../../lib_external/win/${MY_BUILD_TYPE})
    #set(DLL_DIR ../../_molflow/molflow_project/lib/lib)
ELSE()
    # set stuff for other systems

    # link to fedora libraries if EL Linux (Red Hat Enterprise Linux) has been detected
    IF(os_version_suffix STREQUAL ".el7")
        set(LINK_DIR_1 ../../lib_external/linux_fedora)
    ELSE()
        set(LINK_DIR_1 ../../lib_external/linux_debian)
    ENDIF()
ENDIF()

################# CMake Project ############################
# The main options of project                              #
############################################################

project(${PROJECT_NAME} CXX)

# Definition of Macros
add_definitions(
        -DCURL_STATICLIB
        -D_MBCS
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
        -D_CRT_SECURE_NO_DEPRECATE
)
IF (WIN32)
    add_definitions(
            -DWIN
            -D_WINDOWS
            -D_WITHGL
    )
ENDIF()

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

set(SRC_FILES
        ASELoader.cpp
        AddVertex.cpp
        AlignFacet.cpp
        AppUpdater.cpp
        BuildIntersection.cpp
        CollapseSettings.cpp
        CreateShape.cpp
        Distributions.cpp
        ExtrudeFacet.cpp
        FacetCoordinates.cpp
        Facet_shared.cpp
        File.cpp
        FormulaEditor.cpp
        GeometryRender_shared.cpp
        GeometryViewer_shared.cpp
        Geometry_shared.cpp
        GrahamScan.cpp
        HistogramPlotter.cpp
        HistogramSettings.cpp
        Interface.cpp
        LoadStatus.cpp
        MirrorFacet.cpp
        MirrorVertex.cpp
        MoveFacet.cpp
        MoveVertex.cpp
        ParticleLogger.cpp
        Polygon.cpp
        Process.cpp
        Random.cpp
        RecoveryDialog.cpp
        RotateFacet.cpp
        RotateVertex.cpp
        ScaleFacet.cpp
        ScaleVertex.cpp
        SelectDialog.cpp
        SelectTextureType.cpp
        ShMemory.cpp
        SmartSelection.cpp
        SplitFacet.cpp
        Vector.cpp
        VertexCoordinates.cpp
        Web.cpp
        Worker_shared.cpp
        GeometryConverter.cpp

        # subprocesses
        Distributions.cpp
        IntersectAABB_shared.cpp
        Polygon.cpp
        Random.cpp
        ShMemory.cpp
        Process.cpp
        Vector.cpp
        )

file(GLOB GLOB_FILES
        GLApp/*.cpp
        GLApp/GLChart/*.cpp
        #${HEADER_DIR_3}/*.h
        #${HEADER_DIR_4}/*.h
        )

#[[set(HEADER_FILES
        ASELoader.h
        AddVertex.h
        AlignFacet.h
        AppUpdater.h
        Buffer_shared.h
        BuildIntersection.h
        CollapseSettings.h
        CreateShape.h
        Distributions.h
        ExtrudeFacet.h
        FacetCoordinates.h
        Facet_shared.h
        File.h
        FormulaEditor.h
        GeometryViewer.h
        Geometry_shared.h
        GrahamScan.h
        HistogramPlotter.h
        HistogramSettings.h
        Interface.h
        LoadStatus.h
        MirrorFacet.h
        MirrorVertex.h
        MoveFacet.h
        MoveVertex.h
        ParticleLogger.h
        Polygon.h
        RecoveryDialog.h
        RotateFacet.h
        RotateVertex.h
        ScaleFacet.h
        ScaleVertex.h
        SelectDialog.h
        SelectTextureType.h
        SmartSelection.h
        SMP.h
        SplitFacet.h
        Vector.h
        VertexCoordinates.h
        Worker.h
        GeometryConverter.h
        #${HEADER_DIR_1}/GPUSim/MolflowModelParser.h
        )]]

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES} ${GLOB_FILES}
        )


target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        ${HEADER_DIR_5}
        ${HEADER_DIR_6}
        )

if(NOT MSVC)

    if(APPLE)
        #link to self-build sdl shared lib
        target_link_libraries(${PROJECT_NAME} "-framework AppKit")
        #target_link_libraries(${PROJECT_NAME} "-framework SDL2")

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
        #add_library( libSDL2 SHARED IMPORTED GLOBAL)
        #get_filename_component(ABS_LINK_DIR_2 "${LINK_DIR_2}" REALPATH)
        #set_target_properties( libSDL2 PROPERTIES IMPORTED_LOCATION ${ABS_LINK_DIR_2}/libSDL2-2.0.dylib)
        #target_link_libraries(${PROJECT_NAME} libSDL2) # from ./lib/

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

        target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})

        find_package(X11 REQUIRED)
        target_include_directories(${PROJECT_NAME} PRIVATE ${X11_INCLUDE_DIRS})
    endif()

    find_package(OpenGL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${OPENGL_INCLUDE_DIRS})

    find_package(SDL2 REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})

    find_package(PNG REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${PNG_INCLUDE_DIRS})

    find_package(GSL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${GSL_INCLUDE_DIRS})

    find_package(CURL REQUIRED)
    target_include_directories(${PROJECT_NAME} PRIVATE ${CURL_INCLUDE_DIRS})

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    #GSLCBLAS or BLAS?
    #find_package(GSLCBLAS REQUIRED)
    #target_include_directories(${PROJECT_NAME} PRIVATE ${GSLCBLAS_INCLUDE_DIRS})

    #find_package(stdc++fs REQUIRED)
    #target_include_directories(${PROJECT_NAME} PRIVATE ${stdc++fs_INCLUDE_DIRS})

    target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARY})

    target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${GSL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} ${CURL_LIBRARIES})
    target_link_libraries(${PROJECT_NAME} Threads::Threads)
    target_link_libraries(${PROJECT_NAME} ${X11_LIBRARIES})
    #target_link_libraries(${PROJECT_NAME} ${GSLCBLAS_LIBRARIES})

    #for shared memory
    find_library(LIBRT rt)
    if(LIBRT)
        target_link_libraries(${PROJECT_NAME} ${LIBRT})
    endif()

    # Your-external "mylib", add GLOBAL if the imported library is located in directories above the current.
    if (NOT TARGET libzip)
        add_library( libzip STATIC IMPORTED GLOBAL)
    endif()
    # You can define two import-locations: one for debug and one for release.
    get_filename_component(ABS_LINK_DIR_1 "${LINK_DIR_1}" REALPATH)

    # other static libraries need to be built with similar settings for clang (here: libc++)
    if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
        set_target_properties( libzip PROPERTIES IMPORTED_LOCATION ${ABS_LINK_DIR_1}/libzip_clang.a )
    else()
        set_target_properties( libzip PROPERTIES IMPORTED_LOCATION ${ABS_LINK_DIR_1}/libzip_gcc.a )
    endif()
    target_link_libraries(${PROJECT_NAME} libzip) # from ./lib/

    #use prebuild nfd for GCC on MacOS
    if(APPLE AND "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
        add_library( libnfd STATIC IMPORTED GLOBAL)
        set_target_properties( libnfd PROPERTIES IMPORTED_LOCATION ${ABS_LINK_DIR_1}/libnfd.a )
        target_link_libraries(${PROJECT_NAME} libnfd) # from ./lib/
    endif()

    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
        target_link_libraries(${PROJECT_NAME} c++fs)
    elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
        #don´t add anything for filesystem
    else()
        target_link_libraries(${PROJECT_NAME} stdc++fs)
    endif()

    target_link_libraries(${PROJECT_NAME} pugixml clipper sdl_savepng truncatedgaussian)
    target_link_libraries(${PROJECT_NAME} nativefiledialog)
endif(NOT MSVC)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

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

########### Link & Dependencies ############################
# Add project dependencies and Link to project             #
############################################################
#set_target_properties(${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE C)
