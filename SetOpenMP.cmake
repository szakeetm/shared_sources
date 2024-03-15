if (APPLE)
    #with homebrew it should resolve to
    #Intel: /usr/local/opt/libomp/include, /usr/local/opt/libomp/lib/libomp.dylib
    #ARM: /opt/homebrew/lib/

    SET(OpenMP_SEARCH_PATHS
            ~/Library/Frameworks
            /Library/Frameworks
            /usr/local/opt/libomp
            /usr
            /sw # Fink
            /opt/local # DarwinPorts
            /opt/csw # Blastwave
            /opt/homebrew/opt/libomp # ARM MacOS homebrew
            /opt
            ${OpenMP_PATH}
            )

    FIND_PATH(OpenMP_INCLUDE_DIR omp.h
            HINTS
            $ENV{OMPDIR}
            PATH_SUFFIXES libomp/include include
            PATHS ${OpenMP_SEARCH_PATHS}
            )

    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(PATH_SUFFIXES lib64 lib/x64 lib)
    else()
        set(PATH_SUFFIXES lib/x86 lib)
    endif()

    FIND_LIBRARY(OpenMP_LIBRARY_TEMP
            NAMES omp libomp.dylib
            HINTS
            $ENV{OMPDIR}
            PATH_SUFFIXES ${PATH_SUFFIXES}
            PATHS ${OpenMP_SEARCH_PATHS}
            )

    set(OpenMP_C_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_INCLUDE_DIR}" CACHE INTERNAL "OpenMP flags for Xcode toolchain.")
    set(OpenMP_C_LIB_NAMES "omp" CACHE INTERNAL "OpenMP lib name for Xcode toolchain.")

    set(OpenMP_CXX_FLAGS "-Xpreprocessor -fopenmp -I${OpenMP_INCLUDE_DIR}" CACHE INTERNAL "OpenMP flags for Xcode toolchain.")
    set(OpenMP_CXX_LIB_NAMES "omp" CACHE INTERNAL "OpenMP lib name for Xcode toolchain.")
    set(OpenMP_omp_LIBRARY "${OpenMP_LIBRARY_TEMP}" CACHE INTERNAL "OpenMP lib name for Xcode toolchain.")
endif()