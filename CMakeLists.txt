cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

add_subdirectory(compress)
add_subdirectory(external EXCLUDE_FROM_ALL)

if(NOT NO_INTERFACE)
    add_subdirectory(imgui)
    add_subdirectory(implot)
    add_dependencies(implot imgui)
endif()

# Folders files
set(HEADER_DIR_1 .)
set(HEADER_DIR_2 ./GLApp)
set(HEADER_DIR_3 ./GLApp/GLChart)
set(HEADER_DIR_4 ../include)
set(HEADER_DIR_5 ./Interface)
#set(HEADER_DIR_EXT ../src/)
#set(EXTERNAL_1 ./external)
#set(EXTERNAL_2 ./external)

set(HEADER_DIR_ZIP ./external/ziplib/Source)
set(IMGUI_DIR ./imgui)

#set(COPY_DIR ../copy_to_build/)

IF (WIN32)
    # set stuff for windows

    set(HEADER_DIR_6 ../include/windows_only)
    set(HEADER_DIR_7 ../include/windows_only/png)

    set(LINK_DIR_1 ../lib/win/${MY_BUILD_TYPE})
ELSEIF(APPLE)
    # set stuff for mac os
    if ("${CMAKE_SYSTEM_PROCESSOR}" MATCHES "arm64")
        set(LINK_DIR_1 ../../lib_external/mac_arm)
    else()
        set(LINK_DIR_1 ../../lib_external/mac)
    endif()
ELSE()
    # set stuff for other systems

    # link to fedora libraries if EL Linux (Red Hat Enterprise Linux) has been detected
    IF(os_version_suffix MATCHES "\\.el[1-9]")
        set(LINK_DIR_1 ../lib_external/linux_fedora)
    ELSE()
        set(LINK_DIR_1 ../lib_external/linux_debian)
    ENDIF()
ENDIF()

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
            #-D_WINDOWS
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
        Interface/GlobalSettings_shared.cpp
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
        Interface/CrossSection.cpp
        Interface/ImguiWindow.cpp
        Interface/ImguiMenu.cpp
        Interface/ImguiGlobalSettings.cpp
        Interface/ImguiPerformancePlot.cpp
        Interface/ImguiSidebar.cpp
        Interface/ImguiExtensions.cpp
        Interface/ImguiFacetMove.cpp
        Interface/imgui_stdlib/imgui_stdlib.cpp
        Interface/ImguiPopup.cpp
        Interface/ImguiSmartSelection.cpp
        Interface/ImguiSelectDialog.cpp
        Interface/ImguiSelectTextureType.cpp
        Interface/ImguiSelectFacetByResult.cpp
        Interface/ImguiShortcutManager.cpp
        Interface/ImguiWindowBase.cpp
        Interface/ImguiFormulaEditor.cpp
        Facet_shared.cpp
        Helper/GLProgress_ImGui.cpp
        Helper/GLProgress_ImGui.h


        File.cpp
        Formulas.cpp
        FormulaEvaluator.cpp
        Geometry_shared.cpp
        GrahamScan.cpp
        Web.cpp
        Worker_shared.cpp
        GeometryTools.cpp
        )

set(SIM_FILES
        Distributions.cpp
        ProfileModes.cpp

        # subprocesses
        SimulationFacet.cpp
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
        SimulationModel.cpp
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
        Helper/GLProgress_abstract.cpp
        Helper/GLProgress_abstract.hpp
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
        GLApp/GLFormula.cpp
        GLApp/GLProgress_GUI.cpp
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
        GLApp/GLTypes.cpp
        GLApp/GLChart/AxisPanel.cpp
        GLApp/GLChart/GLAxis.cpp
        GLApp/GLChart/GLChart.cpp
        GLApp/GLChart/GLChartOptions.cpp
        GLApp/GLChart/GLDataView.cpp
        GLApp/GLChart/GLDataViewOptions.cpp
        )

if(NOT NO_INTERFACE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/interface.cmake)
endif()
include(${CMAKE_CURRENT_SOURCE_DIR}/simulator.cmake)

include(${CMAKE_HOME_DIRECTORY}/src_shared/SetOpenMP.cmake)
find_package(OpenMP REQUIRED)