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



IF (WIN32)
    set(OS_NAME "win")
ELSE()
    IF(os_version_suffix STREQUAL ".el7")
        set(OS_NAME "linux_fedora")
    ELSE()
        set(OS_NAME "linux_debian")
    ENDIF()
ENDIF()

# Output Variables
set(OUTPUT_DEBUG ../lib/${OS_NAME}/debug/)
set(OUTPUT_REL ../lib/${OS_NAME}/release/)
# Folders files
set(CPP_DIR_2 ../src_shared)
set(CPP_DIR_3 ../src_shared/GLApp)
set(CPP_DIR_4 ../src_shared/GLApp/GLChart)
set(HEADER_DIR_2 ../src_shared)
set(HEADER_DIR_3 ../src_shared/GLApp)
set(HEADER_DIR_4 ../src_shared/GLApp/GLChart)
set(HEADER_DIR_5 ../include)
set(COPY_DIR ../copy_to_build/)

IF (WIN32)
    # set stuff for windows

    set(CPP_DIR_5 ../src_shared/windows_only)
    set(HEADER_DIR_6 ../include/windows_only)
    set(HEADER_DIR_7 ../include/windows_only/png)

    set(LINK_DIR_1 ../lib/win/${MY_BUILD_TYPE})
    #set(LINK_DIR_2 ../../_molflow/molflow_project/lib/lib)
    #set(LINK_DIR_3 ../../lib_external/win/${MY_BUILD_TYPE})
    #set(DLL_DIR ../../_molflow/molflow_project/lib/lib)
ELSE()
    # set stuff for other systems

    set(CPP_DIR_5 "")
    set(HEADER_DIR_6 "")
    set(HEADER_DIR_7 "")
    set(HEADER_DIR_8 "")

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
add_definitions(-DMOLFLOW -DWIN -DCURL_STATICLIB -DNDEBUG -D_WINDOWS -D_WITHGL -D_CRT_SECURE_NO_DEPRECATE)
add_definitions(
        -DCURL_STATICLIB
        -D_MBCS
        -DMOLFLOW
        -D_CRT_SECURE_NO_WARNINGS
        -D_CRT_NONSTDC_NO_DEPRECATE
)


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

# Messages
message("${PROJECT_NAME}: MAIN PROJECT: ${CMAKE_PROJECT_NAME}")
message("${PROJECT_NAME}: CURR PROJECT: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: CURR BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")
# Messages
message("${PROJECT_NAME}: CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_EXECUTABLE_OUTPUT_DIRECTORY: ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}")


############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

set(SRC_FILES
        ${CPP_DIR_2}/ASELoader.cpp
        ${CPP_DIR_2}/AddVertex.cpp
        ${CPP_DIR_2}/AlignFacet.cpp
        ${CPP_DIR_2}/AppUpdater.cpp
        ${CPP_DIR_2}/BuildIntersection.cpp
        ${CPP_DIR_2}/CollapseSettings.cpp
        ${CPP_DIR_2}/CreateShape.cpp
        ${CPP_DIR_2}/Distributions.cpp
        ${CPP_DIR_2}/ExtrudeFacet.cpp
        ${CPP_DIR_2}/FacetCoordinates.cpp
        ${CPP_DIR_2}/Facet_shared.cpp
        ${CPP_DIR_2}/File.cpp
        ${CPP_DIR_2}/FormulaEditor.cpp
        ${CPP_DIR_2}/GeometryRender_shared.cpp
        ${CPP_DIR_2}/GeometryViewer_shared.cpp
        ${CPP_DIR_2}/Geometry_shared.cpp
        ${CPP_DIR_2}/GrahamScan.cpp
        ${CPP_DIR_2}/HistogramPlotter.cpp
        ${CPP_DIR_2}/HistogramSettings.cpp
        ${CPP_DIR_2}/Interface.cpp
        ${CPP_DIR_2}/LoadStatus.cpp
        ${CPP_DIR_2}/MirrorFacet.cpp
        ${CPP_DIR_2}/MirrorVertex.cpp
        ${CPP_DIR_2}/MoveFacet.cpp
        ${CPP_DIR_2}/MoveVertex.cpp
        ${CPP_DIR_2}/ParticleLogger.cpp
        ${CPP_DIR_2}/Polygon.cpp
        ${CPP_DIR_2}/Process.cpp
        ${CPP_DIR_2}/Random.cpp
        ${CPP_DIR_2}/RecoveryDialog.cpp
        ${CPP_DIR_2}/RotateFacet.cpp
        ${CPP_DIR_2}/RotateVertex.cpp
        ${CPP_DIR_2}/ScaleFacet.cpp
        ${CPP_DIR_2}/ScaleVertex.cpp
        ${CPP_DIR_2}/SelectDialog.cpp
        ${CPP_DIR_2}/SelectTextureType.cpp
        ${CPP_DIR_2}/ShMemory.cpp
        ${CPP_DIR_2}/SmartSelection.cpp
        ${CPP_DIR_2}/SplitFacet.cpp
        ${CPP_DIR_2}/Vector.cpp
        ${CPP_DIR_2}/VertexCoordinates.cpp
        ${CPP_DIR_2}/Web.cpp
        ${CPP_DIR_2}/Worker_shared.cpp
        ${CPP_DIR_2}/GeometryConverter.cpp
        #${CPP_DIR_1}/GPUSim/MolflowModelParser.cpp

        # subprocesses
        ${CPP_DIR_2}/Distributions.cpp
        ${CPP_DIR_2}/IntersectAABB_shared.cpp
        ${CPP_DIR_2}/Polygon.cpp
        ${CPP_DIR_2}/Random.cpp
        ${CPP_DIR_2}/ShMemory.cpp
        ${CPP_DIR_2}/Process.cpp
        ${CPP_DIR_2}/Vector.cpp
        )

set(HEADER_FILES
        ${HEADER_DIR_2}/ASELoader.h
        ${HEADER_DIR_2}/AddVertex.h
        ${HEADER_DIR_2}/AlignFacet.h
        ${HEADER_DIR_2}/AppUpdater.h
        ${HEADER_DIR_2}/Buffer_shared.h
        ${HEADER_DIR_2}/BuildIntersection.h
        ${HEADER_DIR_2}/CollapseSettings.h
        ${HEADER_DIR_2}/CreateShape.h
        ${HEADER_DIR_2}/Distributions.h
        ${HEADER_DIR_2}/ExtrudeFacet.h
        ${HEADER_DIR_2}/FacetCoordinates.h
        ${HEADER_DIR_2}/Facet_shared.h
        ${HEADER_DIR_2}/File.h
        ${HEADER_DIR_2}/FormulaEditor.h
        ${HEADER_DIR_2}/GeometryViewer.h
        ${HEADER_DIR_2}/Geometry_shared.h
        ${HEADER_DIR_2}/GrahamScan.h
        ${HEADER_DIR_2}/HistogramPlotter.h
        ${HEADER_DIR_2}/HistogramSettings.h
        ${HEADER_DIR_2}/Interface.h
        ${HEADER_DIR_2}/LoadStatus.h
        ${HEADER_DIR_2}/MirrorFacet.h
        ${HEADER_DIR_2}/MirrorVertex.h
        ${HEADER_DIR_2}/MoveFacet.h
        ${HEADER_DIR_2}/MoveVertex.h
        ${HEADER_DIR_2}/ParticleLogger.h
        ${HEADER_DIR_2}/Polygon.h
        ${HEADER_DIR_2}/RecoveryDialog.h
        ${HEADER_DIR_2}/RotateFacet.h
        ${HEADER_DIR_2}/RotateVertex.h
        ${HEADER_DIR_2}/ScaleFacet.h
        ${HEADER_DIR_2}/ScaleVertex.h
        ${HEADER_DIR_2}/SelectDialog.h
        ${HEADER_DIR_2}/SelectTextureType.h
        ${HEADER_DIR_2}/SmartSelection.h
        ${HEADER_DIR_2}/SMP.h
        ${HEADER_DIR_2}/SplitFacet.h
        ${HEADER_DIR_2}/Vector.h
        ${HEADER_DIR_2}/VertexCoordinates.h
        ${HEADER_DIR_2}/Worker.h
        ${HEADER_DIR_2}/GeometryConverter.h
        #${HEADER_DIR_1}/GPUSim/MolflowModelParser.h
        )

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SRC_FILES}
        )


target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        ${HEADER_DIR_5}
        ${HEADER_DIR_6}
        ${HEADER_DIR_7}
        ${HEADER_DIR_8}
        )

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
