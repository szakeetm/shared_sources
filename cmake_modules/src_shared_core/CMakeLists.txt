#CMake project file called from src_shared's CMakeLists.txt
#CLI-GUI common parts of src_shared
#Built as static library

set(PROJECT_NAME src_shared_core)

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

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
            -D_WITHGL
            -D_USE_MATH_DEFINES #M_PI etc., needs to be defined before first including <cmath>
    )
ENDIF()

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

set(SIM_FILES
        ${CPP_DIR_SRC_SHARED}/Distributions.cpp
        ${CPP_DIR_SRC_SHARED}/ProfileModes.cpp
        ${CPP_DIR_SRC_SHARED}/SimulationFacet.cpp
        ${CPP_DIR_SRC_SHARED}/Buffer_shared.cpp
        ${CPP_DIR_SRC_SHARED}/Polygon.cpp
        ${CPP_DIR_SRC_SHARED}/Random.cpp
        ${CPP_DIR_SRC_SHARED}/ShMemory.cpp
        ${CPP_DIR_SRC_SHARED}/Process.cpp
        ${CPP_DIR_SRC_SHARED}/ProcessControl.cpp

        ${CPP_DIR_SRC_SHARED}/Vector.cpp
        ${CPP_DIR_SRC_SHARED}/SimulationManager.cpp
        ${CPP_DIR_SRC_SHARED}/SimulationController.cpp
        ${CPP_DIR_SRC_SHARED}/SimulationModel.cpp
        ${CPP_DIR_SRC_SHARED}/FacetData.cpp

        ${CPP_DIR_SRC_SHARED}/SettingsIO.cpp

        ${CPP_DIR_SRC_SHARED}/RayTracing/KDTree.h
        ${CPP_DIR_SRC_SHARED}/RayTracing/KDTree.cpp
        ${CPP_DIR_SRC_SHARED}/RayTracing/BVH.cpp
        ${CPP_DIR_SRC_SHARED}/RayTracing/BVH.h
        ${CPP_DIR_SRC_SHARED}/RayTracing/Ray.h
        ${CPP_DIR_SRC_SHARED}/BoundingBox.cpp
)

set(HELPER_FILES
        ${HELPER_DIR}/StringHelper.cpp
        ${HELPER_DIR}/MathTools.cpp
        ${HELPER_DIR}/GraphicsHelper.cpp
        ${HELPER_DIR}/Chronometer.cpp
        ${HELPER_DIR}/FormatHelper.cpp
        ${HELPER_DIR}/GLProgress_abstract.cpp
        ${CPP_DIR_SRC_SHARED}/AppSettings.cpp
)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SIM_FILES}
        ${HELPER_FILES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
        ${HEADER_DIR_SRC_SHARED}
        ${INTERFACE_DIR} #CameraView, etc. still used by shared gui/cli
        ${HEADER_DIR_EXTERNAL}
)

if(MSVC)

else()

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)
    target_link_libraries(${PROJECT_NAME} PUBLIC Threads::Threads)

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
endif()

# VCPKG or system package manager libraries
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
find_package(cereal CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal::cereal)
find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC pugixml::pugixml)
IF(APPLE)
    include(${CPP_DIR_SRC_SHARED}/cmake_modules/SetOpenMP.cmake)  #Help macOS find OpenMP
ENDIF()
find_package(OpenMP REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)

# Third-party libraries shipped with Molflow
target_link_libraries(${PROJECT_NAME} PUBLIC ziplib)
target_link_libraries(${PROJECT_NAME} PUBLIC clipper2)

#Suppress warnings for external libraries
if(MSVC)
    set(SUPPRESS_WARNINGS_FLAG "/w")
else()
    set(SUPPRESS_WARNINGS_FLAG "-w")
endif()

target_compile_options(clipper2 PUBLIC ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(ziplib PUBLIC ${SUPPRESS_WARNINGS_FLAG})