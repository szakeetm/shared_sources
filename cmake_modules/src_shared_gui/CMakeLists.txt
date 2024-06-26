#This is a separate project, excluded if NO_INTERFACE cmake option is set
#Called from src_shared/CMakeLists.txt

set(PROJECT_NAME src_shared_gui)
project(${PROJECT_NAME} CXX)

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
        ${INTERFACE_DIR}/ImguiParticleLogger.cpp
        ${INTERFACE_DIR}/ImguiMovingParts.cpp
        ${INTERFACE_DIR}/ImguiMeasureForce.cpp
        ${INTERFACE_DIR}/ImguiFacetCoordinates.cpp
        ${INTERFACE_DIR}/ImguiFacetScale.cpp
        ${INTERFACE_DIR}/ImguiFacetMirrorProject.cpp
        ${INTERFACE_DIR}/ImguiFacetRotate.cpp
        ${INTERFACE_DIR}/ImguiFacetAlign.cpp
        ${INTERFACE_DIR}/ImguiFacetExtrude.cpp
        ${INTERFACE_DIR}/ImguiFacetSplit.cpp
        ${INTERFACE_DIR}/ImguiCreateShape.cpp
        ${INTERFACE_DIR}/ImguiBuildIntersection.cpp
        ${INTERFACE_DIR}/ImguiCollapseSettings.cpp
        ${INTERFACE_DIR}/ImguiOutgassingMap.cpp
        ${INTERFACE_DIR}/ImguiVertexCoordinates.cpp
        ${INTERFACE_DIR}/ImguiVertexMove.cpp

        ${INTERFACE_DIR}/ImguiGeometryViewer.cpp
        ${CPP_DIR_SRC_SHARED}/Facet_shared.cpp
        
        ${HELPER_DIR}/GLProgress_ImGui.cpp

        ${CPP_DIR_SRC_SHARED}/Formulas.cpp
        ${CPP_DIR_SRC_SHARED}/FormulaEvaluator.cpp
        ${CPP_DIR_SRC_SHARED}/Geometry_shared.cpp
        ${CPP_DIR_SRC_SHARED}/GrahamScan.cpp
        ${CPP_DIR_SRC_SHARED}/Web.cpp
        ${CPP_DIR_SRC_SHARED}/Worker_shared.cpp
        ${CPP_DIR_SRC_SHARED}/GeometryTools.cpp
        ${CPP_DIR_SRC_SHARED}/ImguiImageManager.cpp
)

if(ENABLE_IMGUI_TESTS)
        list(APPEND INTERFACE_FILES
                ${CPP_DIR_SRC_SHARED}/imguiTesting.cpp
        )
endif()

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

# Add library to build.
add_library(${PROJECT_NAME} STATIC
        ${INTERFACE_FILES}
        ${GLAPP_FILES}
         )

target_include_directories(${PROJECT_NAME} PRIVATE
        ${HEADER_DIR_SRC_SHARED}
        ${GLAPP_DIR}
        ${GLCHART_DIR}
        ${INTERFACE_DIR}
        ${IMGUI_DIR}
        ${IMPLOT_DIR}
        ${HEADER_DIR_EXTERNAL}
)
        # Commenting out this section: since now we include imgui by default, which links against SDL2, no need to do it here (would give a linker warning)
        #find_package(SDL2 CONFIG REQUIRED)
        #target_link_libraries(${PROJECT_NAME}
        #    PRIVATE
        #    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
        #    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        #)

target_link_libraries(${PROJECT_NAME} PUBLIC src_shared_core)

if(MSVC)
    # 1. link against external libs
    target_link_libraries(${PROJECT_NAME} PUBLIC
            bzip2.lib
            lzma.lib
            opengl32#.lib
            user32.lib
            shell32.lib
            ole32.lib
            )
ELSE() #not MSVC

    if(APPLE)
        #link to self-built sdl shared lib
        target_link_libraries(${PROJECT_NAME} PRIVATE "-framework AppKit")
    else()
        # Use the package PkgConfig to detect GTK+ headers/library files
        FIND_PACKAGE(PkgConfig REQUIRED)
        PKG_CHECK_MODULES(GTK3 REQUIRED gtk+-3.0)

        # Setup CMake to use GTK+, tell the compiler where to look for headers
        # and to the linker where to look for libraries
        INCLUDE_DIRECTORIES(${GTK3_INCLUDE_DIRS})
        LINK_DIRECTORIES(${GTK3_LIBRARY_DIRS})

        # Add other flags to the compiler
        ADD_DEFINITIONS(${GTK3_CFLAGS_OTHER})

        target_link_libraries(${PROJECT_NAME} PUBLIC ${GTK3_LIBRARIES})

        find_package(X11 REQUIRED)
        target_include_directories(${PROJECT_NAME} SYSTEM PUBLIC ${X11_INCLUDE_DIRS})
        target_link_libraries(${PROJECT_NAME} PUBLIC ${X11_LIBRARIES})
    endif()

    #external libraries from our project
    message(Shared CMAKE_LIBRARY_OUTPUT_DIRECTORY: ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
    target_link_directories(${PROJECT_NAME} PRIVATE ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif() #NOT MSVC

#target_precompile_headers(${PROJECT_NAME} PRIVATE ${CPP_DIR_SRC_SHARED}/graphicsLibraries.h)

#vcpkg or system-wide libraries
find_package(OpenGL REQUIRED)
find_package(CURL REQUIRED)
find_package(cereal CONFIG REQUIRED)
find_package(GLEW REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE GLEW::GLEW OpenGL::GLU)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)
target_link_libraries(${PROJECT_NAME} PRIVATE cereal::cereal)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENGL_LIBRARIES})
find_package(Stb REQUIRED)
message("-- Found STB " ${Stb_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} PUBLIC ${Stb_INCLUDE_DIR})


#Shipped libraries
target_link_libraries(${PROJECT_NAME} PUBLIC sdl_savepng)
target_link_libraries(${PROJECT_NAME} PUBLIC nativefiledialog-extended-molflow-wrapped)
target_link_libraries(${PROJECT_NAME} PUBLIC imgui)
target_link_libraries(${PROJECT_NAME} PUBLIC implot)

#Suppress warnings for external libraries
if(MSVC)
    set(SUPPRESS_WARNINGS_FLAG "/w")
else()
    set(SUPPRESS_WARNINGS_FLAG "-w")
endif()

target_compile_options(clipper2 PRIVATE ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(sdl_savepng PRIVATE ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(ziplib PRIVATE ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(imgui PRIVATE ${SUPPRESS_WARNINGS_FLAG})
target_compile_options(implot PRIVATE ${SUPPRESS_WARNINGS_FLAG})

