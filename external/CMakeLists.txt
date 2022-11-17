# First process subdirectories related to external projects
set(CMAKE_CXX_FLAGS_BAK "${CMAKE_CXX_FLAGS}")
if(NOT MSVC)
    # for persistent warnings from ziplib
    add_compile_options(
            -Wno-conversion
            -Wno-sign-conversion
            -Wno-deprecated-declarations
    )
endif()

add_subdirectory(ziplib)
add_subdirectory(fmt)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BAK}")