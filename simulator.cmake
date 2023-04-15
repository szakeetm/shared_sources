################# CMake Project ############################
# The main options of project                              #
############################################################

#CMake project file called from src_shared's CMakeLists.txt

set(PROJECT_NAME shared_simulator)
project(${PROJECT_NAME} CXX)

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${SIM_FILES}
        ${HELPER_FILES}
        )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_1}
        ${HEADER_DIR_2}
        ${HEADER_DIR_3}
        ${HEADER_DIR_4}
        ${HEADER_DIR_ZIP}
        )


target_include_directories(${PROJECT_NAME} PUBLIC
        ${HEADER_DIR_5}
        ${HEADER_DIR_6}
        ${HEADER_DIR_7}
        )

include(SetOpenMP.cmake)
find_package(OpenMP REQUIRED)
if(OpenMP_CXX_FOUND)
    message(STATUS "Detected OpenMP version: ${OpenMP_CXX_VERSION}")
    target_link_libraries(${PROJECT_NAME} PRIVATE OpenMP::OpenMP_CXX)
endif()

if(NOT MSVC)
    find_package(GSL REQUIRED)
    target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${GSL_INCLUDE_DIRS})

    set(THREADS_PREFER_PTHREAD_FLAG ON)
    find_package(Threads REQUIRED)

    target_link_libraries(${PROJECT_NAME} PUBLIC ${GSL_LIBRARIES})
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

target_link_libraries(${PROJECT_NAME} PUBLIC fmtlib_src) # header include
target_link_libraries(${PROJECT_NAME} PUBLIC fmt)
target_link_libraries(${PROJECT_NAME} PUBLIC cereal)
target_link_libraries(${PROJECT_NAME} PUBLIC ziplib)
target_link_libraries(${PROJECT_NAME} PUBLIC clipper2)

######################### Flags ############################
# Defines Flags for Windows and Linux                      #
############################################################

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)