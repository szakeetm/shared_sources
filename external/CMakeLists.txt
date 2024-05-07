# First process subdirectories related to external projects
set(CMAKE_CXX_FLAGS_BAK "${CMAKE_CXX_FLAGS}") #suppress warnings

if(NOT MSVC)
    # for persistent warnings from ziplib
    add_compile_options(
            -Wno-conversion
            -Wno-sign-conversion
            -Wno-deprecated-declarations
    )
endif()

#Suppress warnings for external libraries
if(MSVC)
    set(SUPPRESS_WARNINGS_FLAG "/w")
else()
    set(SUPPRESS_WARNINGS_FLAG "-w")
endif()

add_subdirectory(ziplib)
target_compile_options(ziplib PRIVATE ${SUPPRESS_WARNINGS_FLAG})

add_subdirectory(Clipper2Lib)
target_compile_options(clipper2 PRIVATE ${SUPPRESS_WARNINGS_FLAG})

add_subdirectory(truncated_normal)

if(NOT NO_INTERFACE)
    add_subdirectory(SDL_SavePNG)
    target_compile_options(sdl_savepng PRIVATE ${SUPPRESS_WARNINGS_FLAG})
    add_subdirectory(NativeFileDialog)
    target_compile_options(nativefiledialog-extended-molflow-wrapped PRIVATE ${SUPPRESS_WARNINGS_FLAG})
    add_subdirectory(imgui)
    target_compile_options(imgui PRIVATE ${SUPPRESS_WARNINGS_FLAG})
    add_subdirectory(implot)
    target_compile_options(implot PRIVATE ${SUPPRESS_WARNINGS_FLAG})
endif()

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS_BAK}") #restore warnings