cmake_minimum_required(VERSION 3.12.2 FATAL_ERROR)

##################### Variables ############################
# Change if you want modify path or other values           #
############################################################

add_subdirectory(compress)
add_subdirectory(external EXCLUDE_FROM_ALL)

# Folders files
set(HEADER_DIR_EXTERNAL ./external) #applied in src_shared_gui.cmake and src_shared_core.cmake
set(IMGUI_DIR ${HEADER_DIR_EXTERNAL}/imgui) #applied in src_shared_gui.cmake
set(IMPLOT_DIR ${HEADER_DIR_EXTERNAL}/implot) #applied in src_shared_gui.cmake
set(INTERFACE_DIR Interface)

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
    )
ENDIF()

############### Files & Targets ############################
# Files of project and target to build                     #
############################################################

set(INTERFACE_FILES
        ${INTERFACE_DIR}/AddVertex.cpp
        ${INTERFACE_DIR}/AlignFacet.cpp
        ${INTERFACE_DIR}/AppUpdater.cpp
        ${INTERFACE_DIR}/BuildIntersection.cpp
        ${INTERFACE_DIR}/CollapseSettings.cpp
        ${INTERFACE_DIR}/ConvergencePlotter.cpp
        ${INTERFACE_DIR}/CreateShape.cpp
        ${INTERFACE_DIR}/ExtrudeFacet.cpp
        ${INTERFACE_DIR}/FacetCoordinates.cpp
        ${INTERFACE_DIR}/FormulaEditor.cpp
        ${INTERFACE_DIR}/GeometryRender_shared.cpp
        ${INTERFACE_DIR}/GeometryViewer_shared.cpp
        ${INTERFACE_DIR}/GlobalSettings_shared.cpp
        ${INTERFACE_DIR}/HistogramPlotter.cpp
        ${INTERFACE_DIR}/HistogramSettings.cpp
        ${INTERFACE_DIR}/Interface.cpp
        ${INTERFACE_DIR}/LoadStatus.cpp
        ${INTERFACE_DIR}/MirrorFacet.cpp
        ${INTERFACE_DIR}/MirrorVertex.cpp
        ${INTERFACE_DIR}/MoveFacet.cpp
        ${INTERFACE_DIR}/MoveVertex.cpp
        ${INTERFACE_DIR}/ParticleLogger.cpp
        ${INTERFACE_DIR}/RecoveryDialog.cpp
        ${INTERFACE_DIR}/RotateFacet.cpp
        ${INTERFACE_DIR}/RotateVertex.cpp
        ${INTERFACE_DIR}/ScaleFacet.cpp
        ${INTERFACE_DIR}/ScaleVertex.cpp
        ${INTERFACE_DIR}/SelectDialog.cpp
        ${INTERFACE_DIR}/SelectFacetByResult.cpp
        ${INTERFACE_DIR}/SelectTextureType.cpp
        ${INTERFACE_DIR}/SmartSelection.cpp
        ${INTERFACE_DIR}/SplitFacet.cpp
        ${INTERFACE_DIR}/VertexCoordinates.cpp
        ${INTERFACE_DIR}/CrossSection.cpp
        ${INTERFACE_DIR}/ImguiWindow.cpp
        ${INTERFACE_DIR}/ImguiMenu.cpp
        ${INTERFACE_DIR}/ImguiGlobalSettings.cpp
        ${INTERFACE_DIR}/ImguiPerformancePlot.cpp
        ${INTERFACE_DIR}/ImguiSidebar.cpp
        ${INTERFACE_DIR}/ImguiExtensions.cpp
        ${INTERFACE_DIR}/ImguiFacetMove.cpp
        ${INTERFACE_DIR}/ImguiPopup.cpp
        ${INTERFACE_DIR}/ImguiSmartSelection.cpp
        ${INTERFACE_DIR}/ImguiSelectDialog.cpp
        ${INTERFACE_DIR}/ImguiSelectTextureType.cpp
        ${INTERFACE_DIR}/ImguiSelectFacetByResult.cpp
        ${INTERFACE_DIR}/ImguiShortcutManager.cpp
        ${INTERFACE_DIR}/ImguiWindowBase.cpp
        ${INTERFACE_DIR}/ImguiFormulaEditor.cpp
        ${INTERFACE_DIR}/ImguiConvergencePlotter.cpp
        ${INTERFACE_DIR}/ImguiTexturePlotter.cpp
        ${INTERFACE_DIR}/ImguiProfilePlotter.cpp
        ${INTERFACE_DIR}/ImguiHistogramPlotter.cpp
        ${INTERFACE_DIR}/ImguiTextureScaling.cpp

        Facet_shared.cpp
        
        ${HELPER_DIR}/GLProgress_ImGui.cpp

        File.cpp
        Formulas.cpp
        FormulaEvaluator.cpp
        Geometry_shared.cpp
        GrahamScan.cpp
        Web.cpp
        Worker_shared.cpp
        GeometryTools.cpp
        )

if(ENABLE_IMGUI_TESTS)
        list(APPEND INTERFACE_FILES
                imguiTesting.cpp
        )
endif()

set(SIM_FILES
        Distributions.cpp
        ProfileModes.cpp
        SimulationFacet.cpp
        Buffer_shared.cpp
        Distributions.cpp
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

        SettingsIO.cpp

        RayTracing/KDTree.h
        RayTracing/KDTree.cpp
        RayTracing/BVH.cpp
        RayTracing/BVH.h
        RayTracing/Ray.h
        BoundingBox.cpp
        )

set(HELPER_FILES
        ${HELPER_DIR}/StringHelper.cpp
        ${HELPER_DIR}/MathTools.cpp
        ${HELPER_DIR}/GraphicsHelper.cpp
        ${HELPER_DIR}/Chronometer.cpp
        ${HELPER_DIR}/FormatHelper.cpp
        ${HELPER_DIR}/GLProgress_abstract.cpp
        AppSettings.cpp
        )

set(GLAPP_FILES
        ${GLAPP_DIR}/GLApp.cpp
        ${GLAPP_DIR}/GLButton.cpp
        ${GLAPP_DIR}/GLColorBox.cpp
        ${GLAPP_DIR}/GLCombo.cpp
        ${GLAPP_DIR}/GLComponent.cpp
        ${GLAPP_DIR}/GLContainer.cpp
        ${GLAPP_DIR}/GLFont.cpp
        ${GLAPP_DIR}/GLGradient.cpp
        ${GLAPP_DIR}/GLIcon.cpp
        ${GLAPP_DIR}/GLInputBox.cpp
        ${GLAPP_DIR}/GLLabel.cpp
        ${GLAPP_DIR}/GLList.cpp
        ${GLAPP_DIR}/GLMatrix.cpp
        ${GLAPP_DIR}/GLMenu.cpp
        ${GLAPP_DIR}/GLMenuBar.cpp
        ${GLAPP_DIR}/GLMessageBox.cpp
        ${GLAPP_DIR}/GLFormula.cpp
        ${GLAPP_DIR}/GLProgress_GUI.cpp
        ${GLAPP_DIR}/GLSaveDialog.cpp
        ${GLAPP_DIR}/GLScrollBar.cpp
        ${GLAPP_DIR}/GLSpinner.cpp
        ${GLAPP_DIR}/GLSprite.cpp
        ${GLAPP_DIR}/GLTabWindow.cpp
        ${GLAPP_DIR}/GLTextField.cpp
        ${GLAPP_DIR}/GLTitledPanel.cpp
        ${GLAPP_DIR}/GLToggle.cpp
        ${GLAPP_DIR}/GLToolkit.cpp
        ${GLAPP_DIR}/GLUnitDialog.cpp
        ${GLAPP_DIR}/GLWindow.cpp
        ${GLAPP_DIR}/GLWindowManager.cpp
        ${GLAPP_DIR}/GLTypes.cpp
        ${GLAPP_DIR}/GLChart/AxisPanel.cpp
        ${GLAPP_DIR}/GLChart/GLAxis.cpp
        ${GLAPP_DIR}/GLChart/GLChart.cpp
        ${GLAPP_DIR}/GLChart/GLChartOptions.cpp
        ${GLAPP_DIR}/GLChart/GLDataView.cpp
        ${GLAPP_DIR}/GLChart/GLDataViewOptions.cpp
        )

include(${CMAKE_CURRENT_SOURCE_DIR}/src_shared_core.cmake) #CLI project, build as library

if(NOT NO_INTERFACE)
    include(${CMAKE_CURRENT_SOURCE_DIR}/src_shared_gui.cmake) #GUI project, build as library
    find_package(CURL CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE ${CURL_LIBRARIES})
endif()

include(${CMAKE_HOME_DIRECTORY}/src_shared/SetOpenMP.cmake)  #Help macOS find OpenMP
find_package(OpenMP REQUIRED)