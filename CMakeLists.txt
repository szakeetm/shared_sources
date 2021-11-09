cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

# First process subdirectories related to external projects
add_subdirectory(compress)
add_subdirectory(PugiXML)
add_subdirectory(TruncatedGaussian)
add_subdirectory(ziplib)
add_subdirectory(fmt)

if(NOT NO_INTERFACE)
    add_subdirectory(SDL_SavePNG)
    add_subdirectory(Clipper)
    add_subdirectory(NativeFileDialog)
    add_subdirectory(imgui)
    add_subdirectory(implot)
    add_dependencies(implot imgui)
endif()

# Project
#get_filename_component(PROJECT_DIR "${CMAKE_CURRENT_SOURCE_DIR}" ABSOLUTE)
#set(DEPENDENCIES_DIR ${PROJECT_DIR}/dependencies)

# Folders files
set(HEADER_DIR_1 .)
set(HEADER_DIR_2 ./GLApp)
set(HEADER_DIR_3 ./GLApp/GLChart)
set(HEADER_DIR_4 ../include)
set(HEADER_DIR_5 ./Interface)
set(HEADER_DIR_EXT ../src/)
set(IMGUI_DIR ./imgui)

set(COPY_DIR ../copy_to_build/)

IF (WIN32)
    # set stuff for windows

    set(HEADER_DIR_6 ../include/windows_only)
    set(HEADER_DIR_7 ../include/windows_only/png)

    set(LINK_DIR_1 ../lib/win/${MY_BUILD_TYPE})
    #set(LINK_DIR_2 ../../_molflow/molflow_project/lib/lib)
    #set(LINK_DIR_3 ../../lib_external/win/${MY_BUILD_TYPE})
    #set(DLL_DIR ../../_molflow/molflow_project/lib/lib)
ELSEIF(APPLE)
    # set stuff for mac os
    set(LINK_DIR_1 ../lib_external/mac)
ELSE()
    # set stuff for other systems

    # link to fedora libraries if EL Linux (Red Hat Enterprise Linux) has been detected
    IF(os_version_suffix STREQUAL ".el7")
        set(LINK_DIR_1 ../lib_external/linux_fedora)
    ELSE()
        set(LINK_DIR_1 ../lib_external/linux_debian)
    ENDIF()
ENDIF()

############## Artefacts Output ############################
# Defines outputs , depending BUILD TYPE                   #
############################################################

#[[if(CMAKE_BUILD_TYPE MATCHES Debug|RelWithDebInfo)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_DEBUG}")
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
    set(CMAKE_EXECUTABLE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${OUTPUT_REL}")
endif()]]

#[[# Messages
message("${PROJECT_NAME}: MAIN PROJECT: ${CMAKE_PROJECT_NAME}")
message("${PROJECT_NAME}: CURR PROJECT: ${CMAKE_CURRENT_SOURCE_DIR}")
message("${PROJECT_NAME}: CURR BIN DIR: ${CMAKE_CURRENT_BINARY_DIR}")
# Messages
message("${PROJECT_NAME}: CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_ARCHIVE_OUTPUT_DIRECTORY: ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}")
message("${PROJECT_NAME}: CMAKE_EXECUTABLE_OUTPUT_DIRECTORY: ${CMAKE_EXECUTABLE_OUTPUT_DIRECTORY}")]]

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

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

set(INTERFACE_FILES
        Interface/AddVertex.cpp
        Interface/AlignFacet.cpp
        Interface/AppUpdater.cpp
        Interface/BuildIntersection.cpp
        Interface/CollapseSettings.cpp
        Interface/ConvergencePlotter.cpp
        Interface/CreateShape.cpp
        Interface/ExtrudeFacet.cpp
        Interface/FacetCoordinates.cpp
        Interface/FormulaEditor.cpp
        Interface/GeometryRender_shared.cpp
        Interface/GeometryViewer_shared.cpp
        Interface/HistogramPlotter.cpp
        Interface/HistogramSettings.cpp
        Interface/Interface.cpp
        Interface/LoadStatus.cpp
        Interface/MirrorFacet.cpp
        Interface/MirrorVertex.cpp
        Interface/MoveFacet.cpp
        Interface/MoveVertex.cpp
        Interface/ParticleLogger.cpp
        Interface/RecoveryDialog.cpp
        Interface/RotateFacet.cpp
        Interface/RotateVertex.cpp
        Interface/ScaleFacet.cpp
        Interface/ScaleVertex.cpp
        Interface/SelectDialog.cpp
        Interface/SelectFacetByResult.cpp
        Interface/SelectTextureType.cpp
        Interface/SmartSelection.cpp
        Interface/SplitFacet.cpp
        Interface/VertexCoordinates.cpp
        Interface/ImguiWindow.cpp
        Interface/ImguiMenu.cpp
        Interface/ImguiAABB.cpp
        Interface/ImguiGlobalSettings.cpp
        Interface/ImguiExtensions.cpp
        Interface/transfer_function_widget.cpp

        ASELoader.cpp
        Facet_shared.cpp
        File.cpp
        Formulas.cpp
        FormulaEvaluator.cpp
        Geometry_shared.cpp
        GrahamScan.cpp
        Web.cpp
        Worker_shared.cpp
        GeometryTools.cpp
        AABB.cpp #only used for visualisation at this point
        )

set(SIM_FILES
        Distributions.cpp
        ProfileModes.cpp

        # subprocesses
        Buffer_shared.cpp
        Distributions.cpp
        #IntersectAABB_shared.cpp
        Polygon.cpp
        Random.cpp
        ShMemory.cpp
        Process.cpp
        ProcessControl.cpp

        Vector.cpp
        SimulationManager.cpp
        SimulationController.cpp
        FacetData.cpp
        FacetData.h

        SettingsIO.cpp

        RayTracing/KDTree.h
        RayTracing/KDTree.cpp
        RayTracing/BVH.cpp
        RayTracing/BVH.h
        RayTracing/Ray.cpp
        RayTracing/Ray.h
        RayTracing/Primitive.h
        BoundingBox.cpp
        BoundingBox.h
        RayTracing/RTHelper.h
        )

set(HELPER_FILES
        Helper/StringHelper.cpp
        Helper/MathTools.cpp
        Helper/GraphicsHelper.cpp
        Helper/Chronometer.cpp
        Helper/FormatHelper.cpp
        Helper/ConsoleLogger.h
        Helper/CircularBuffer.h #template
        AppSettings.cpp
        )

set(GL_FILES
        GLApp/GLApp.cpp
        GLApp/GLButton.cpp
        GLApp/GLColorBox.cpp
        GLApp/GLCombo.cpp
        GLApp/GLComponent.cpp
        GLApp/GLContainer.cpp
        GLApp/GLFont.cpp
        GLApp/GLGradient.cpp
        GLApp/GLIcon.cpp
        GLApp/GLInputBox.cpp
        GLApp/GLLabel.cpp
        GLApp/GLList.cpp
        GLApp/GLMatrix.cpp
        GLApp/GLMenu.cpp
        GLApp/GLMenuBar.cpp
        GLApp/GLMessageBox.cpp
        GLApp/GLParser.cpp
        GLApp/GLProgress.cpp
        GLApp/GLSaveDialog.cpp
        GLApp/GLScrollBar.cpp
        GLApp/GLSpinner.cpp
        GLApp/GLSprite.cpp
        GLApp/GLTabWindow.cpp
        GLApp/GLTextField.cpp
        GLApp/GLTitledPanel.cpp
        GLApp/GLToggle.cpp
        GLApp/GLToolkit.cpp
        GLApp/GLUnitDialog.cpp
        GLApp/GLWindow.cpp
        GLApp/GLWindowManager.cpp
        GLApp/GLChart/AxisPanel.cpp
        GLApp/GLChart/GLAxis.cpp
        GLApp/GLChart/GLChart.cpp
        GLApp/GLChart/GLChartOptions.cpp
        GLApp/GLChart/GLDataView.cpp
        GLApp/GLChart/GLDataViewOptions.cpp
        )

#include(${CMAKE_CURRENT_SOURCE_DIR}/SetOpenMP.cmake)

include(SetOpenMP.cmake)
find_package(OpenMP REQUIRED)

if(NOT NO_INTERFACE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/interface.cmake)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/simulator.cmake)

