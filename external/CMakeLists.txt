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

add_subdirectory(TruncatedGaussian)
add_subdirectory(ziplib)
add_subdirectory(Clipper2Lib)

if(NOT NO_INTERFACE)
    add_subdirectory(SDL_SavePNG)
    add_subdirectory(NativeFileDialog)
    target_compile_options(nativefiledialog-extended-molflow-wrapped PRIVATE ${SUPPRESS_WARNINGS_FLAG})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BAK}")