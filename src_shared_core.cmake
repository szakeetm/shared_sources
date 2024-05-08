################# CMake Project ############################
# The main options of project                              #
############################################################

#CMake project file called from src_shared's CMakeLists.txt
#CLI-GUI common parts of src_shared

set(PROJECT_NAME src_shared_core)
project(${PROJECT_NAME} CXX)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SIM_FILES}
        ${HELPER_FILES}
         )

target_include_directories(${PROJECT_NAME} PUBLIC
        .
        ${GLAPP_DIR}
        ${GLCHART_DIR}
        ${HEADER_DIR_SRC_SHARED}
        ${INTERFACE_DIR}
        ${HEADER_DIR_EXTERNAL}
)

include(SetOpenMP.cmake)  #Help macOS find OpenMP
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "Detected OpenMP version: ${OpenMP_CXX_VERSION}")
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

if(NOT MSVC)

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

endif(NOT MSVC)

# VCPKG or system package manager libraries
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC fmt::fmt)
find_package(cereal CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal::cereal)
find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PUBLIC pugixml::pugixml)

# Third-party libraries shipped with Molflow
target_link_libraries(${PROJECT_NAME} PUBLIC ziplib)
target_link_libraries(${PROJECT_NAME} PUBLIC clipper2)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)