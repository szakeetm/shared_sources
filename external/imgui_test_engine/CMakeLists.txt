set(PROJECT_NAME imgui_test_engine)

################################################################################
# Source groups
################################################################################
set(imgui_test_engine
    "imgui_capture_tool.cpp"
    "imgui_capture_tool.h"
    "imgui_te_context.cpp"
    "imgui_te_context.h"
    "imgui_te_coroutine.cpp"
    "imgui_te_coroutine.h"
    "imgui_te_engine.cpp"
    "imgui_te_engine.h"
    "imgui_te_exporters.cpp"
    "imgui_te_exporters.h"
    "imgui_te_imconfig.h"
    "imgui_te_internal.h"
    "imgui_te_perftool.cpp"
    "imgui_te_perftool.h"
    "imgui_te_ui.cpp"
    "imgui_te_ui.h"
    "imgui_te_utils.cpp"
    "imgui_te_utils.h"
)
source_group("imgui_test_engine" FILES ${imgui_test_engine})

set(thirdparty__Str
    "thirdparty/Str/Str.h"
)
source_group("thirdparty\\Str" FILES ${thirdparty__Str})

set(thirdparty__stb
    "thirdparty/stb/imstb_image_write.h"
)
source_group("thirdparty\\stb" FILES ${thirdparty__stb})

set(ALL_FILES
    ${imgui_test_engine}
    ${thirdparty__Str}
    ${thirdparty__stb}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE imgui_test_engine)

################################################################################
# Output directory
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        OUTPUT_DIRECTORY_DEBUG   "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
        OUTPUT_DIRECTORY_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/$<CONFIG>/"
    )
endif()
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
    )
endif()
################################################################################
# Include directories
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/..;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../imgui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/backends;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../imgui_test_suite/thirdparty/implot"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_include_directories(${PROJECT_NAME} PUBLIC
        "${CMAKE_CURRENT_SOURCE_DIR}/..;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../imgui;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../../imgui/backends;"
        "${CMAKE_CURRENT_SOURCE_DIR}/../imgui_test_suite/thirdparty/implot"
    )
endif()

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_UNICODE;"
        "UNICODE;"
        "IMGUI_USER_CONFIG=\"imgui_test_suite/imgui_test_suite_imconfig.h\""
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "_UNICODE;"
        "UNICODE;"
        "IMGUI_USER_CONFIG=\"imgui_test_suite/imgui_test_suite_imconfig.h\""
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy;
                /GS-
            >
            /W4;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            $(ClFlags);
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_compile_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Debug>:
                /Od
            >
            $<$<CONFIG:Release>:
                /O2;
                /Oi;
                /Gy;
                /GS-
            >
            /W4;
            ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
            $(ClFlags);
            ${DEFAULT_CXX_EXCEPTION_HANDLING}
        )
    endif()
    if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:CONSOLE
        )
    elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
        target_link_options(${PROJECT_NAME} PRIVATE
            $<$<CONFIG:Release>:
                /OPT:REF;
                /OPT:ICF
            >
            /SUBSYSTEM:CONSOLE
        )
    endif()
endif()

################################################################################
# Dependencies
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "d3d11;"
        "d3dcompiler;"
        "dxgi"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    set(ADDITIONAL_LIBRARY_DEPENDENCIES
        "d3d11;"
        "d3dcompiler;"
        "dxgi"
    )
endif()
target_link_libraries(${PROJECT_NAME} PUBLIC "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "Win32")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "$ENV{DXSDK_DIR}/Lib/x86"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_link_directories(${PROJECT_NAME} PUBLIC
        "$ENV{DXSDK_DIR}/Lib/x64"
    )
endif()

