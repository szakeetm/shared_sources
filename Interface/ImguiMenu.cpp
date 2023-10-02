/*
Program:     MolFlow+ / Synrad+
Description: Monte Carlo simulator for ultra-high vacuum and synchrotron radiation
Authors:     Jean-Luc PONS / Roberto KERSEVAN / Marton ADY / Pascal BAEHR
Copyright:   E.S.R.F / CERN
Website:     https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
*/

#include "ImguiMenu.h"
#include "imgui/imgui.h"
#include <imgui/IconsFontAwesome5.h>
#include <imgui/IconsMaterialDesign.h>
#include <Helper/MathTools.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"

#include "VertexCoordinates.h"
#include "FacetCoordinates.h"
#include "SmartSelection.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "SelectFacetByResult.h"
#include "FormulaEditor.h"
#include "ConvergencePlotter.h"
#include "HistogramPlotter.h"
#include "ParticleLogger.h"
#include "ScaleFacet.h"
#include "MirrorFacet.h"
#include "RotateFacet.h"
#include "AlignFacet.h"
#include "ExtrudeFacet.h"
#include "SplitFacet.h"
#include "CreateShape.h"
#include "BuildIntersection.h"
#include "MoveVertex.h"
#include "ScaleVertex.h"
#include "MirrorVertex.h"
#include "RotateVertex.h"
#include "AddVertex.h"

#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "../Helper/StringHelper.h"
#include "Worker.h"

#include "GeometryTools.h"

#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"

#if defined(MOLFLOW)

#include "../../src/MolFlow.h"
#include "../../molflow/src/Interface/TexturePlotter.h"
#include "../../molflow/src/Interface/ProfilePlotter.h"
#include "../../molflow/src/Interface/TextureScaling.h"
#include "../../molflow/src/Interface/Movement.h"
#include "../../molflow/src/Interface/MeasureForce.h"
#include "../../molflow/src/Interface/OutgassingMapWindow.h"
#include "../../molflow/src/Interface/PressureEvolution.h"
#include "../../molflow/src/Interface/TimewisePlotter.h"
#include "../../molflow/src/Interface/TimeSettings.h"
#include "../../molflow/src/Interface/MomentsEditor.h"
#include "../../molflow/src/Interface/ParameterEditor.h"
#include "../../molflow/src/Interface/ImportDesorption.h"

#include "../../molflow/src/versionId.h"

#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

#if defined(MOLFLOW)
extern MolFlow *mApp;
#endif

#if defined(SYNRAD)
extern SynRad*mApp;
#endif

extern char fileSaveFilters[];

InterfaceGeometry* interfGeom;

namespace ImMenu {

enum Menu_Event
{
    IMENU_FILE_NEW = 1 << 0, // binary 0001
    IMENU_FILE_LOAD = 1 << 1, // binary 0010
    IMENU_FILE_SAVE = 1 << 2, // binary 0100
    IMENU_FILE_SAVEAS = 1 << 3,  // binary 1000
    IMENU_FILE_INSERTGEO = 1 << 4,
    IMENU_FILE_INSERTGEO_NEWSTR = 1 << 5,
    IMENU_FILE_EXPORT_SELECTION = 1 << 6,
    IMENU_FILE_EXPORTPROFILES = 1 << 7,
    IMENU_FILE_LOADRECENT = 1 << 8,
    IMENU_FILE_EXIT = 1 << 9,
    IMENU_EDIT_TSCALING = 1 << 10,
    IMENU_TOOLS_FORMULAEDITOR = 1 << 11,
    IMENU_EDIT_GLOBALSETTINGS = 1 << 12
};

static bool showPopup = false;
static std::string modalMsg = "";
static std::string modalTitle = "";
static size_t selectionToBeRemovedId = -1;

static std::string saveResponse = "None";

void Popup(std::string msg, std::string title) {
    showPopup = true;
    modalTitle = title;
    modalMsg = msg;
}

void Popup(std::string msg) {
    Popup(msg, "Popup");
}

void DoLoadFile() {
    if (mApp->worker.IsRunning())
        mApp->worker.Stop_Public();
    {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->LoadFile("");
    }
}

void ShowPopup() {
    ImGuiIO& io = ImGui::GetIO();
    float txtW = ImGui::CalcTextSize(" ").x;
    float msgW = ImGui::CalcTextSize(" ").x;
    ImGui::CaptureKeyboardFromApp(true);
    ImGui::SetNextWindowSize(ImVec2(70*txtW,0));
    if (ImGui::BeginPopupModal(modalTitle.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize || ImGuiWindowFlags_NoResize))
    {
        ImGui::PlaceAtRegionCenter(modalMsg);
        ImGui::TextWrapped(modalMsg);
        ImGui::PlaceAtRegionCenter("OK");
        if (ImGui::Button("OK", ImVec2(12*txtW,0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyEsc] || io.KeysDown[ImGui::keyNumEnter])
        {
            ImGui::CloseCurrentPopup();
            modalTitle = "";
        }
        ImGui::EndPopup();
    }
}

void InsertGeometryMenuPress(bool newStr) {
    
    if (interfGeom->IsLoaded()) {
        if (mApp->worker.IsRunning())
            mApp->worker.Stop_Public();
        {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->InsertGeometry(newStr, "");
        }
    }
    else Popup("No geometry loaded.","No geometry");
}

void SaveMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->SaveFile();
    }
    else Popup("No geometry loaded.", "No geometry");
}

void SaveAsMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->SaveFileAs();
    }
    else Popup("No geometry loaded.", "No geometry");
}

void ExportSelectedFacetsMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportSelection();
}

void ExportSelectedProfilesMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportProfiles();
}

void ExportTextures(int a, int b) {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportTextures(a, b);
}

#ifdef MOLFLOW //TODO replace with polimorphism
void ShowSubmenuExportTextures(bool coord=false) {
    if(ImGui::MenuItem("Cell Area (cm^2)")) {
        ExportTextures(coord, 0);
    } // imgui does not support superscript
    if(ImGui::MenuItem("# of MC Hits")) {
        ExportTextures(coord, 1);
    }
    if(ImGui::MenuItem("Impingement rate (1/s/m^2)")) {
        ExportTextures(coord, 2);
    }
    if(ImGui::MenuItem("Particle density (1/m^3)")) {
        ExportTextures(coord, 3);
    }
    if(ImGui::MenuItem("Gas density (kg/m^3)")) {
        ExportTextures(coord, 4);
    }
    if(ImGui::MenuItem("Pressure (mbar)")) {
        ExportTextures(coord, 5);
    }
    if(ImGui::MenuItem("Avg. Velocity (m/s)")) {
        ExportTextures(coord, 6);
    }
    if(ImGui::MenuItem("Velocity vector (m/s)")) {
        ExportTextures(coord, 7);
    }
    if(ImGui::MenuItem("# of velocity vectors")) {
        ExportTextures(coord, 8);
    }
}

void ImportDesorptionFromSYNFileMenuPress() {
    if (interfGeom->IsLoaded()) {
        if (!mApp->importDesorption) mApp->importDesorption = new ImportDesorption();
        mApp->importDesorption->SetGeometry(interfGeom, &mApp->worker);
        mApp->importDesorption->SetVisible(true);
    }
    else Popup("No geometry loaded.", "No geometry");

}
#endif // MOLFLOW

void NewGeometry() {
    if (mApp->worker.IsRunning())
        mApp->worker.Stop_Public();
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->EmptyGeometry();
}

void DoSave() {
    LockWrapper myLock(mApp->imguiRenderLock);
    std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
    if (!fn.empty()) {
        GLProgress_Abstract prg = GLProgress_Abstract("Saving"); // placeholder
        try {
            mApp->worker.SaveGeometry(fn, prg);
            mApp->changedSinceSave = false;
            mApp->UpdateTitle();
            mApp->AddRecent(fn);
        }
        catch (const std::exception& e) {
            std::string errMsg = ("%s\nFile:%s", e.what(), fn.c_str());
            mApp->imWnd->popup.OpenImMsgBox("Error", errMsg, { std::make_shared<MyButtonInt>("OK", buttonOk) });
            mApp->RemoveRecent(fn.c_str());
        }
    }
}

void DoLoadSelected(std::string file) {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->LoadFile(file);
}

static void ShowMenuFile() {
    if(ImGui::MenuItem(ICON_FA_PLUS "  New, empty geometry")){
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); NewGeometry(); };
            auto N = []() -> void { NewGeometry(); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            NewGeometry();
        }
    }

    if(ImGui::MenuItem(ICON_FA_FILE_IMPORT "  Load", "Ctrl+O")){
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); DoLoadFile(); };
            auto N = []() -> void { DoLoadFile(); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            DoLoadFile();
        }
    }
    if (mApp->recentsList.empty()) {
        ImGui::BeginDisabled();
        ImGui::MenuItem("Load recent");
        ImGui::EndDisabled();
    }
    else if(ImGui::BeginMenu(ICON_FA_ARROW_CIRCLE_LEFT "  Load recent")){
        for (int i = mApp->recentsList.size() - 1; i >= 0; i--) {
            if (ImGui::MenuItem(mApp->recentsList[i])) {
                if (mApp->changedSinceSave) {
                    auto Y = [](std::string selection) -> void { DoSave(); DoLoadSelected(selection); };
                    auto N = [](std::string selection) -> void { DoLoadSelected(selection); };
                    mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFuncStr>("Yes", Y, mApp->recentsList[i]) ,std::make_shared<MyButtonFuncStr>("No", N,mApp->recentsList[i]), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
                }
                else {
                    DoLoadSelected(mApp->recentsList[i]);
                }
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Insert geometry")) {
        if(ImGui::MenuItem("To current structure")){
            InsertGeometryMenuPress(false);
        }
        if(ImGui::MenuItem("To new structure")){
            InsertGeometryMenuPress(true);
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if(ImGui::MenuItem(ICON_FA_SAVE "  Save", "Ctrl+S")){
        if (mApp->worker.GetGeometry()->IsLoaded())
            SaveMenuPress();
    }
    if(ImGui::MenuItem(ICON_FA_SAVE "  Save as")){
        if (mApp->worker.GetGeometry()->IsLoaded())
            SaveAsMenuPress();
    }
    ImGui::Separator();

    if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "  Export selected facets")) {
        ExportSelectedFacetsMenuPress();
    }

    if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "  Export selected profiles")) {
        ExportSelectedProfilesMenuPress();
    }

    #ifdef MOLFLOW // TODO: Polimorph instead of macro
    if (ImGui::BeginMenu(ICON_FA_FILE_EXPORT "  Export selected textures")) {
        if (ImGui::BeginMenu("Facet By Facet")) {
            ShowSubmenuExportTextures(false);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("By X, Y, Z coordinates")) {
            ShowSubmenuExportTextures(true);
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem(ICON_FA_FILE_IMPORT "  Import desorption from SYN file")) {
        ImportDesorptionFromSYNFileMenuPress();
    }
    #endif //MOLFLOW
    ImGui::Separator();
    if (ImGui::MenuItem(ICON_FA_TIMES "  Quit", "Alt+F4")) {
        exit(0);
    }
}

static std::string inputValue = "1.0";
static void ShowSelectionModals() {
    ImGuiIO& io = ImGui::GetIO();
    Worker &worker = mApp->worker;
    InterfaceGeometry *interfGeom = worker.GetGeometry();

    if (ImGui::BeginPopupModal("Select large facets without hits", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::CaptureKeyboardFromApp(true);
        double largeAreaThreshold;

        ImGui::InputText("Min.area (cm\262)", &inputValue);

        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            if (!Util::getNumber(&largeAreaThreshold, inputValue)) {
                ImGui::OpenPopup("Incorrect Value");
            }
            else {
                interfGeom->UnselectAll();
                for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                    if (interfGeom->GetFacet(i)->facetHitCache.nbMCHit == 0 &&
                        interfGeom->GetFacet(i)->sh.area >= largeAreaThreshold)
                        interfGeom->SelectFacet(i);
                interfGeom->UpdateSelection();
                mApp->UpdateFacetParams(true);
                ImGui::CloseCurrentPopup();
                showPopup = false;
            }
        }
        if (ImGui::BeginPopupModal("Incorrect Value", NULL,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Incorrect value");
            if (ImGui::Button("OK") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

    else if (ImGui::BeginPopupModal("Select non planar facets", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::CaptureKeyboardFromApp(true);
        double planarityThreshold = 1e-5;

        ImGui::InputText("Planarity larger than:", &inputValue);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            if (!Util::getNumber(&planarityThreshold, inputValue)) {
                ImGui::OpenPopup("Incorrect Value");
            } else {
                interfGeom->UnselectAll();
                std::vector<size_t> nonPlanarFacetids = interfGeom->GetNonPlanarFacetIds(planarityThreshold);
                for (const auto& i : nonPlanarFacetids)
                    interfGeom->SelectFacet(i);
                interfGeom->UpdateSelection();
                mApp->UpdateFacetParams(true);
                ImGui::CloseCurrentPopup();
                showPopup = false;
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

    else if (ImGui::BeginPopupModal("Enter selection name", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::CaptureKeyboardFromApp(true);
        static std::string selectionName = "Selection #" + std::to_string(mApp->selections.size()+1);
        ImGui::InputText("Selection name", &selectionName, 128);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            mApp->AddSelection(selectionName);
            ImGui::CloseCurrentPopup();
            selectionName = "Selection #" + std::to_string(mApp->selections.size() + 1);
            showPopup = false;
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

    else if (ImGui::BeginPopupModal("Clear all selections ?", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            mApp->ClearAllSelections();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

    else if (ImGui::BeginPopupModal("Clear memorized selection?", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::TextWrapped(modalMsg);
        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            try {
                mApp->selections.erase(mApp->selections.begin() + (selectionToBeRemovedId ));
                ImGui::CloseCurrentPopup();
            }
            catch (std::exception e) {
                ImGui::EndPopup();
                throw e;
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

static void ShowMenuSelection(std::string& openModalName) {
    static SmartSelection *smartSelection = nullptr;
    static SelectDialog *selectDialog = nullptr;
    static SelectTextureType *selectTextureType = nullptr;
    static SelectFacetByResult *selectFacetByResult = nullptr;

    Worker &worker = mApp->worker;
    InterfaceGeometry *interfGeom = worker.GetGeometry();

    if (ImGui::MenuItem("Smart Select facets...", "ALT+S")) {
        if (!smartSelection) smartSelection = new SmartSelection(worker.GetGeometry(), &worker);
        smartSelection->SetVisible(true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select All Facets", "CTRL+A")) {
        interfGeom->SelectAll();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select by Facet Number...", "ALT+N")) {
        if (!selectDialog) selectDialog = new SelectDialog(worker.GetGeometry());
        selectDialog->SetVisible(true);
    }

#ifdef MOLFLOW
        // TODO: Different for Synrad?
    if (ImGui::MenuItem("Select Sticking", "")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (!interfGeom->GetFacet(i)->sh.stickingParam.empty() ||
                (interfGeom->GetFacet(i)->sh.sticking != 0.0 && !interfGeom->GetFacet(i)->IsTXTLinkFacet()))
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
#endif // MOLFLOW

    if (ImGui::MenuItem("Select Transparent", "")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (
            #if defined(MOLFLOW)
            !interfGeom->GetFacet(i)->sh.opacityParam.empty() ||
            #endif
            (interfGeom->GetFacet(i)->sh.opacity != 1.0 && interfGeom->GetFacet(i)->sh.opacity != 2.0))
            {
                interfGeom->SelectFacet(i);
            }
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    if (ImGui::MenuItem("Select 2 sided")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->sh.is2sided)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Texture")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->sh.isTextured)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select by Texture type...")) {
        if (!selectTextureType) selectTextureType = new SelectTextureType(&worker);
        selectTextureType->SetVisible(true);
    }
    if (ImGui::MenuItem("Select Profile")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->sh.isProfile)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Select Abs > 0")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
            if (interfGeom->GetFacet(i)->facetHitCache.nbAbsEquiv > 0)
                interfGeom->SelectFacet(i);
        }
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Hit > 0")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->facetHitCache.nbMCHit > 0)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select large with no hits...")) {
        openModalName="Select large facets without hits";
        showPopup = true;
    }
    if (ImGui::MenuItem("Select by facet result...")) {
        if (!selectFacetByResult) selectFacetByResult = new SelectFacetByResult(&worker);
        selectFacetByResult->SetVisible(true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select link facets")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)

            if (interfGeom->GetFacet(i)->sh.superDest != 0)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select teleport facets")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)

            if (interfGeom->GetFacet(i)->sh.teleportDest != 0)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select non planar facets")) {
        openModalName="Select non planar facets";
        showPopup = true;
    }
    if (ImGui::MenuItem("Select non simple facets")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)

            if (interfGeom->GetFacet(i)->nonSimple)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    if (ImGui::MenuItem("Invert selection", "CTRL+I")) {
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            interfGeom->GetFacet(i)->selected = !interfGeom->GetFacet(i)->selected;
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Memorize selection to")) {
        if (ImGui::MenuItem("Add new...", "CTRL+W")) {
            openModalName="Enter selection name";
            showPopup = true;
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                mApp->selections[i].facetIds = interfGeom->GetSelectedFacets(); // TODO: Modal to ask for confirmation
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Select memorized", !mApp->selections.empty())) {
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (i <= 8) {
                char shortcut[32];
                sprintf(shortcut, "ALT+%llu", 1 + i);
                if (ImGui::MenuItem(mApp->selections[i].name.c_str(), shortcut)) {
                    mApp->SelectSelection(i);
                }
            } else {
                if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                    mApp->SelectSelection(i);
                }
            }
        }

        ImGui::Separator();
        if (ImGui::MenuItem("Select previous", "ALT+F11")) {
            mApp->SelectSelection(Previous(mApp->idSelection, mApp->selections.size()));
        }
        if (ImGui::MenuItem("Select next", "ALT+F12")) {
            mApp->SelectSelection(Next(mApp->idSelection, mApp->selections.size()));
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Clear memorized")) {
        if (ImGui::MenuItem("Clear All")) {
            openModalName="Clear all selections ?";
            showPopup = true;
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                openModalName = "Clear memorized selection?";
                modalMsg = "Are you sure you wish to forget selection %s", mApp->selections[i].name;
                selectionToBeRemovedId = i;
                showPopup = true;
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    // TODO: Extract Molflow only entries
#if defined(MOLFLOW)
    if (ImGui::MenuItem("Select Desorption")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->sh.desorbType != DES_NONE)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Outgassing Map")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (interfGeom->GetFacet(i)->hasOutgassingFile)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Reflective")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
            InterfaceFacet *f = interfGeom->GetFacet(i);
            if (f->sh.desorbType == DES_NONE && f->sh.sticking == 0.0 && f->sh.opacity > 0.0)
                interfGeom->SelectFacet(i);
        }
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
#endif

    if (ImGui::MenuItem("Shortcut test", "CTRL+Q")) {
        openModalName="testmod";
        showPopup = true;
    }
}

void FormulaEditorMenuPress() {
    
    if (!interfGeom->IsLoaded()) {
        Popup("No geometry loaded.", "No geometry");
    }
    else if (!mApp->formulaEditor || !mApp->formulaEditor->IsVisible()) {
        SAFE_DELETE(mApp->formulaEditor);
        mApp->formulaEditor = new FormulaEditor(&mApp->worker, mApp->appFormulas);
        mApp->formulaEditor->Refresh();
        // Load values on init
        mApp->appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
        mApp->formulaEditor->UpdateValues();
        // ---
        mApp->formulaEditor->SetVisible(true);
    }
}
void ConvergencePlotterMenuPress() {
    if (!mApp->convergencePlotter)
        mApp->convergencePlotter = new ConvergencePlotter(&mApp->worker, mApp->appFormulas);
    else {
        if (!mApp->convergencePlotter->IsVisible()) {
            auto* newConv = new ConvergencePlotter(*mApp->convergencePlotter);
            //newConv->SetViews(convergencePlotter->GetViews());
            SAFE_DELETE(mApp->convergencePlotter);
            mApp->convergencePlotter = newConv;
        }
    }
    mApp->convergencePlotter->Display(&mApp->worker);
}
#ifdef MOLFLOW //TODO replace with polimorphism
void TexturePlotterMenuPress() {
    if (!mApp->texturePlotter) mApp->texturePlotter = new TexturePlotter();
    mApp->texturePlotter->Display(&mApp->worker);
}

void ProfilePlotterMenuPress() {
    if (!mApp->profilePlotter) mApp->profilePlotter = new ProfilePlotter(&mApp->worker);
    mApp->profilePlotter->Display(&mApp->worker);
}
#endif //MOLFLOW

void HistogramPlotterMenuPress() {
    if (!mApp->histogramPlotter || !mApp->histogramPlotter->IsVisible()) {
        SAFE_DELETE(mApp->histogramPlotter);
        mApp->histogramPlotter = new HistogramPlotter(&mApp->worker);
    }
    mApp->histogramPlotter->SetVisible(true);
}
void TextureScalingMenuPress() {
    if (!mApp->textureScaling || !mApp->textureScaling->IsVisible()) {
        SAFE_DELETE(mApp->textureScaling);
        mApp->textureScaling = new TextureScaling();
        mApp->textureScaling->Display(&mApp->worker, mApp->viewer);
    }
}
void ParticleLoggerMenuPress() {
    if (!mApp->particleLogger || !mApp->particleLogger->IsVisible()) {
        SAFE_DELETE(mApp->particleLogger);
        
        mApp->particleLogger = new ParticleLogger(interfGeom, &mApp->worker);
    }
    mApp->particleLogger->UpdateStatus();
    mApp->particleLogger->SetVisible(true);
}

void TakeScreenshotMenuPress() {
    std::ostringstream tmp_ss;

    char buf[80];
    time_t now = time(nullptr);
    struct tm tstruct = *localtime(&now);
    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
    // for more information about date/time format
    strftime(buf, sizeof(buf), "%Y_%m_%d__%H_%M_%S", &tstruct);

    tmp_ss << buf << "_" << mApp->worker.GetCurrentShortFileName();
    std::string oriName = tmp_ss.str();
    tmp_ss.str("");
    tmp_ss.clear();
    for (char c : oriName) {
        bool basic_ascii = ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z'));
        if (basic_ascii) tmp_ss << c;
        else tmp_ss << '_';
    }
    std::string asciiName = tmp_ss.str();
    tmp_ss.str("");
    tmp_ss.clear();
    tmp_ss << "Screenshots/" << asciiName << ".png";


    std::filesystem::create_directory("Screenshots"); //Doesn't do anything if already exists

    int x, y, width, height;
    mApp->viewer[mApp->curViewer]->GetBounds(&x, &y, &width, &height);

    int leftMargin = 4; //Left bewel
    int rightMargin = 0;
    int topMargin = 0;
    int bottomMargin = 28; //Toolbar

    mApp->viewer[mApp->curViewer]->RequestScreenshot(tmp_ss.str(), leftMargin, topMargin,
        width - leftMargin - rightMargin,
        height - topMargin - bottomMargin);
}

#ifdef MOLFLOW //TODO replace with polimorphism
void MovingPartsMenuPress() {
    
    if (!mApp->movement) mApp->movement = new Movement(interfGeom, &mApp->worker);
    mApp->movement->Update();
    mApp->movement->SetVisible(true);
}
void MeasureForcesMenuPress() {
    
    if (!mApp->measureForces) mApp->measureForces = new MeasureForce(interfGeom, &mApp->worker);
    mApp->measureForces->Update();
    mApp->measureForces->SetVisible(true);
}
#endif //MOLFLOW

static void ShowMenuTools() {
    if (ImGui::MenuItem("Formula editor", "ALT+F")) {
        FormulaEditorMenuPress(); // TODO: replace with Toggle ImGui Formula Editor
    }
    if (ImGui::MenuItem("Convergence Plotter ...", "ALT+C")) {
        ConvergencePlotterMenuPress();  // TODO: replace with Toggle ImGui Convergence Plotter
    }
    ImGui::Separator();

#if defined(MOLFLOW)
    if (ImGui::MenuItem("Texture Plotter ...", "ALT+T")) {
        TexturePlotterMenuPress();
    }
    if (ImGui::MenuItem("Profile Plotter ...", "ALT+P")) {
        ProfilePlotterMenuPress();
    }
#endif
    ImGui::Separator();
    if (ImGui::MenuItem("Histogram Plotter...")) {
        HistogramPlotterMenuPress();
    }
    if (ImGui::MenuItem("Texture scaling...", "CTRL+D")) {
        TextureScalingMenuPress();
    }
    if (ImGui::MenuItem("Particle logger...")) {
        ParticleLoggerMenuPress();
    }
    //if (ImGui::MenuItem("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER)){}
    if (ImGui::MenuItem("Global Settings ...")) {
        mApp->imWnd->show_global_settings = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Take screenshot", "CTRL+R")) {
        TakeScreenshotMenuPress();
    }

    ImGui::Separator();
#ifdef MOLFLOW //TODO replace with polimorphism
    if (ImGui::MenuItem("Moving parts...")) {
        MovingPartsMenuPress();
    }
    if (ImGui::MenuItem("Measure forces...")) {
        MeasureForcesMenuPress();
    }
#endif
}

void FacetDeleteMenuPress(std::string& openmodalName) {
    
    auto selectedFacets = interfGeom->GetSelectedFacets();
    if (selectedFacets.empty()) return; //Nothing selected

    openmodalName = "Delete selected facets?";
    showPopup = true;
}
void SwapNormalMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    
    if (mApp->AskToReset()) {
        interfGeom->SwapNormal();
        // Send to sub process
        mApp->worker.MarkToReload();
    }
}
void ShiftIndicesMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    
    if (mApp->AskToReset()) {
        interfGeom->ShiftVertex();
        // Send to sub process
        mApp->worker.MarkToReload();
    }
}
void FacetCoordinatesMenuPress() {
    if (!mApp->facetCoordinates) mApp->facetCoordinates = new FacetCoordinates();
    mApp->facetCoordinates->Display(&mApp->worker);
    mApp->facetCoordinates->SetVisible(true);
}

void FacetScaleMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->scaleFacet) mApp->scaleFacet = new ScaleFacet(interfGeom, &mApp->worker);

        mApp->scaleFacet->SetVisible(true);

    }
    else {
        Popup("No geometry");
    }
}
void FacetMirrorProjectMenuPress() {
    
    if (!mApp->mirrorFacet) mApp->mirrorFacet = new MirrorFacet(interfGeom, &mApp->worker);
    mApp->mirrorFacet->SetVisible(true);
}
void FacetRotateMenuPress() {
    
    if (!mApp->rotateFacet) mApp->rotateFacet = new RotateFacet(interfGeom, &mApp->worker);
    mApp->rotateFacet->SetVisible(true);
}
void AlignToMenuPress() {
    
    if (!mApp->alignFacet) mApp->alignFacet = new AlignFacet(interfGeom, &mApp->worker);
    mApp->alignFacet->MemorizeSelection();
    mApp->alignFacet->SetVisible(true);
}
void ExtrudeMenuPress() {
    
    if (!mApp->extrudeFacet || !mApp->extrudeFacet->IsVisible()) {
        SAFE_DELETE(mApp->extrudeFacet);
        mApp->extrudeFacet = new ExtrudeFacet(interfGeom, &mApp->worker);
    }
    mApp->extrudeFacet->SetVisible(true);
}
void SplitMenuPress() {
    
    if (!mApp->splitFacet || !mApp->splitFacet->IsVisible()) {
        SAFE_DELETE(mApp->splitFacet);
        mApp->splitFacet = new SplitFacet(interfGeom, &mApp->worker);
        mApp->splitFacet->SetVisible(true);
    }
}
void CreateShapeMenuPress() {
    
    if (!mApp->createShape) mApp->createShape = new CreateShape(interfGeom, &mApp->worker);
    mApp->createShape->SetVisible(true);
}

void TransitionBetween2MenuPress() {
    
    if (interfGeom->GetNbSelectedFacets() != 2) {
        Popup("Select exactly 2 facets");
        return;
    }
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        interfGeom->CreateLoft();
    }
    mApp->worker.MarkToReload();
    mApp->UpdateModelParams();
    mApp->UpdateFacetlistSelected();
    mApp->UpdateViewers();
}
void BuildIntersectionMenuPress() {
    
    if (!mApp->buildIntersection || !mApp->buildIntersection->IsVisible()) {
        SAFE_DELETE(mApp->buildIntersection);
        mApp->buildIntersection = new BuildIntersection(interfGeom, &mApp->worker);
        mApp->buildIntersection->SetVisible(true);
    }
}
void CollapseMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        mApp->DisplayCollapseDialog();
    }
    else Popup("No geometry");
}

void ExplodeMenuPress(std::string &openpopupName) {
    openpopupName = "Explode?";
    showPopup = true;
}

void RevertMenuPress() {
    
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        interfGeom->RevertFlippedNormals();
        // Send to sub process
        mApp->worker.MarkToReload();
    }
}

void TriangulateMenuPress(std::string& openmodalName) {
    
    auto selectedFacets = interfGeom->GetSelectedFacets();
    if (selectedFacets.empty()) return;
    openmodalName = "Triangulate?";
    showPopup = true;
}

#ifdef MOLFLOW // TODO switch to polimorphism
void ConvertToOutgassingMapMenuPress() {
    if (!mApp->outgassingMapWindow) mApp->outgassingMapWindow = new OutgassingMapWindow();
    mApp->outgassingMapWindow->Display(&mApp->worker);
}
#endif
void ExplodePupup(std::string& openmodalName) {
    float txtW = ImGui::CalcTextSize(" ").x;
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::BeginPopupModal("Explode?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("Are you sure you want to explode selected facets?");
        if (ImGui::Button("OK", ImVec2(12*txtW, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter])
        {
            LockWrapper myLock(mApp->imguiRenderLock);
            if (mApp->AskToReset()) {
                
                int err;
                try {
                    err = interfGeom->ExplodeSelected();
                }
                catch (const std::exception& e) {
                    openmodalName = "Error";
                    modalMsg = "Error Exploding";
                    ImGui::CloseCurrentPopup();
                }
                if (err == -1) {
                    openmodalName = "Error";
                    modalMsg = "Empty selection";
                    ImGui::CloseCurrentPopup();
                }
                else if (err == -2) {
                    openmodalName = "Error";
                    modalMsg = "All selected facets must have a mesh with boudary correction enabled";
                    ImGui::CloseCurrentPopup();
                }
                else if (err == 0) {
                    mApp->UpdateModelParams();
                    mApp->UpdateFacetParams(true);
                    // Send to sub process
                    mApp->worker.MarkToReload();
                    ImGui::CloseCurrentPopup();
                    showPopup = false;
                }
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(12 * txtW, 0)) || showPopup==false || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}

void ErrorPopup() {
    float txtW = ImGui::CalcTextSize(" ").x;
    auto io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(txtW * (modalMsg.length() + 10), 0));
    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings)) {
        ImGui::TextWrapped(modalMsg);
        if (ImGui::Button("  OK  ") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}

void TriangulatePopup() {
    ImGuiIO& io = ImGui::GetIO();
    if (ImGui::BeginPopupModal("Triangulate?")) {
        ImGui::Text("Triangulation can't be undone. Are you sure?");
        if (ImGui::Button("   OK   ") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
            LockWrapper myLock(mApp->imguiRenderLock);
            if (mApp->AskToReset()) {
                
                auto selectedFacets = interfGeom->GetSelectedFacets();
                auto prg = GLProgress_GUI("Triangulating", "Triangulating");
                prg.SetVisible(true);
                GeometryTools::PolygonsToTriangles(interfGeom, selectedFacets, prg);
            }
            mApp->worker.MarkToReload();
            mApp->UpdateModelParams();
            mApp->UpdateFacetlistSelected();
            mApp->UpdateViewers();
        } ImGui::SameLine();
        if (ImGui::Button("   Cancel   ") || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}


static void ShowMenuFacet(std::string& openmodalName) {
    if (ImGui::MenuItem("Delete", "CTRL+DEL")) {
        FacetDeleteMenuPress(openmodalName);
    }
    if (ImGui::MenuItem("Swap normal", "CTRL+N")) {
        SwapNormalMenuPress();
    }
    if (ImGui::MenuItem("Shift indices", "CTRL+H")) {
        ShiftIndicesMenuPress();
    }
    if (ImGui::MenuItem("Facet coordinates ...")) {
        FacetCoordinatesMenuPress();
    }
    if (ImGui::MenuItem("Move ...")) {
        mApp->imWnd->show_facet_move = true;
    }
    if (ImGui::MenuItem("Scale ...")) {
        FacetScaleMenuPress();
    }
    if (ImGui::MenuItem("Mirror / Project ...")) {
        FacetMirrorProjectMenuPress();
    }
    if (ImGui::MenuItem("Rotate ...")) {
        FacetRotateMenuPress();
    }
    if (ImGui::MenuItem("Align to ...")) {
        AlignToMenuPress();
    }
    if (ImGui::MenuItem("Extrude ...")) {
        ExtrudeMenuPress();
    }
    if (ImGui::MenuItem("Split ...")) {
        SplitMenuPress();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Create shape...")) {
        CreateShapeMenuPress();
    }

    if (ImGui::BeginMenu("Create two facets' ...")) {
        if (ImGui::BeginMenu("Difference")) {
            if (ImGui::MenuItem("Auto (non-zero)")) {
                LockWrapper myLock(mApp->imguiRenderLock);
                mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 2);
            }
            if (ImGui::MenuItem("First - Second")) {
                LockWrapper myLock(mApp->imguiRenderLock);
                mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 0);
            }
            if (ImGui::MenuItem("Second - First")) {
                LockWrapper myLock(mApp->imguiRenderLock);
                mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 1);
            }
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Union")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Union);
        }
        if (ImGui::MenuItem("Intersection")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Intersection);
        }
        if (ImGui::MenuItem("XOR")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->CreateOfTwoFacets(Clipper2Lib::ClipType::Xor);
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Transition between 2")) {
        TransitionBetween2MenuPress();
    }
    if (ImGui::MenuItem("Build intersection...")) {
        BuildIntersectionMenuPress();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Collapse ...")) {
        CollapseMenuPress();
    }
    if (ImGui::MenuItem("Explode")) {
        ExplodeMenuPress(openmodalName);
    }
    if (ImGui::MenuItem("Revert flipped normals (old geometries)")) {
        RevertMenuPress();
    }
    if (ImGui::MenuItem("Triangulate")) {
        TriangulateMenuPress(openmodalName);
    }
#ifdef MOLFLOW //convert to polimophism
    if (ImGui::MenuItem("Convert to outgassing map...")) {
        ConvertToOutgassingMapMenuPress();
    }
#endif
}

void ConvexHullMenuPress(std::string& openmodalName) {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (interfGeom->IsLoaded()) {
        if (interfGeom->GetNbSelectedVertex() != 3) {
            openmodalName = "Error";
            showPopup = true;
            modalMsg = "Select exactly 3 vertices";
            return;
        }
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }

    if (mApp->AskToReset()) {
        try {
            interfGeom->CreatePolyFromVertices_Convex();
        }
        catch (const std::exception& e) {
            openmodalName = "Error";
            showPopup = true;
            modalMsg = "Error creating polygon";
        }
        mApp->worker.MarkToReload();
    }
}
void SelectionOrderMenuPress(std::string& openmodalName) {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        try {
            
            interfGeom->CreatePolyFromVertices_Order();
        }
        catch (const std::exception& e) {
            openmodalName = "Error";
            showPopup = true;
            modalMsg = "Error creating polygon";
        }
        mApp->worker.MarkToReload();
    }
}
void ClearIsolatedMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    
    interfGeom->DeleteIsolatedVertices(false);
    mApp->UpdateModelParams();
    if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
    if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
    interfGeom->BuildGLList();
}
void RemoveSelectedMenuPress(std::string& openmodalName) {
    
    if (interfGeom->IsLoaded()) {
        openmodalName = "Remove Vertices";
        showPopup = true;
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }
}
void VertexCoordinatesMenuPress() {
    if (!mApp->vertexCoordinates) mApp->vertexCoordinates = new VertexCoordinates();
    mApp->vertexCoordinates->Display(&mApp->worker);
    mApp->vertexCoordinates->SetVisible(true);
}
void VertexMoveMenuPress(std::string& openmodalName) {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->moveVertex) mApp->moveVertex = new MoveVertex(interfGeom, &mApp->worker);
        mApp->moveVertex->SetVisible(true);
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }
}
void VertexScaleMenuPress(std::string& openmodalName) {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->scaleVertex) mApp->scaleVertex = new ScaleVertex(interfGeom, &mApp->worker);
        mApp->scaleVertex->SetVisible(true);
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }
}
void VertexMirrorProjectMenuPress() {
    
    if (!mApp->mirrorVertex) mApp->mirrorVertex = new MirrorVertex(interfGeom, &mApp->worker);
    mApp->mirrorVertex->SetVisible(true);
}
void VertexRotateMenuPress() {
    
    if (!mApp->rotateVertex) mApp->rotateVertex = new RotateVertex(interfGeom, &mApp->worker);
    mApp->rotateVertex->SetVisible(true);
}
void VertexAddNewMenuPress(std::string& openmodalName) {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->addVertex) mApp->addVertex = new AddVertex(interfGeom, &mApp->worker);
        mApp->addVertex->SetVisible(true);
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }
}

void VertexCoplanarMenuPress(std::string& openmodalName) {
    char* input;
    if (interfGeom->IsLoaded()) {
        if (interfGeom->GetNbSelectedVertex() != 3) {
            openmodalName = "Error";
            showPopup = true;
            modalMsg = "Can't define plane, Select exactly 3 vertices";
        }
        else {
            openmodalName = "Select Coplanar Vertices";
            showPopup = true;
        }
    }
    else {
        openmodalName = "Error";
        showPopup = true;
        modalMsg = "No geometry loaded";
    }
}

void SelectCoplanarVerticesPopup(std::string& openmodalName) {
    float txtW = ImGui::CalcTextSize(" ").x;
    ImGui::SetNextWindowSize(ImVec2(75 * txtW,0));
    if (ImGui::BeginPopupModal("Select Coplanar Vertices")) {
        ImGuiIO& io = ImGui::GetIO();
        std::string input = "1.0";
        ImGui::InputText("Tolerance(cm)", &input);
        if (ImGui::Button("   OK   ") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            if (!Util::getNumber(&mApp->coplanarityTolerance, input)) {
                openmodalName = "Error";
                showPopup = true;
                modalMsg = "Invalid Number";
                ImGui::CloseCurrentPopup();
            }
            else {
                try { mApp->viewer[mApp->curViewer]->SelectCoplanar(mApp->coplanarityTolerance); }
                catch (const std::exception& e) {
                    openmodalName = "Error";
                    showPopup = true;
                    modalMsg = "Error selecting coplanar vertices";
                }
                ImGui::CloseCurrentPopup();
                showPopup = false;
            }
        } ImGui::SameLine();
        if (ImGui::Button("   Cancel   ") || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

}

#ifdef MOLFLOW
void RemoveVertexPopup() {
    if (ImGui::BeginPopupModal("Remove Vertices")) {
        ImGuiIO& io = ImGui::GetIO();
        ImGui::Text("Remove Selected vertices?");
        ImGui::Text("This will also removed the facets containing them!");
        if (ImGui::Button("   OK   ") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
            LockWrapper myLock(mApp->imguiRenderLock);
            if (mApp->AskToReset()) {
                
                if (mApp->worker.IsRunning()) mApp->worker.Stop_Public();
                interfGeom->RemoveSelectedVertex();
                //worker.CalcTotalOutgassing();
                interfGeom->Rebuild(); //Will recalculate facet parameters
                mApp->UpdateModelParams();
                if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
                if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
                if (mApp->profilePlotter) mApp->profilePlotter->Refresh();
                if (mApp->pressureEvolution) mApp->pressureEvolution->Refresh();
                if (mApp->timewisePlotter) mApp->timewisePlotter->Refresh();
                if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
                if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
                // Send to sub process
                mApp->worker.MarkToReload();
            }
        } ImGui::SameLine();
        if (ImGui::Button("   Cancel   ") || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}
#endif
static void ShowMenuVertex(std::string& openmodalName) {
    if (ImGui::BeginMenu("Create Facet from Selected")) {
        if (ImGui::MenuItem("Convex Hull", "ALT+V")) {
            ConvexHullMenuPress(openmodalName);
        }
        if (ImGui::MenuItem("Keep selection order")) {
            SelectionOrderMenuPress(openmodalName);
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Clear isolated")) {
        ClearIsolatedMenuPress();
    }
#ifdef MOLFLOW
    if (ImGui::MenuItem("Remove selected")) {
        RemoveSelectedMenuPress(openmodalName);
    }
#endif
    if (ImGui::MenuItem("Vertex coordinates...")) {
        VertexCoordinatesMenuPress();
    }
    if (ImGui::MenuItem("Move...")) {
        VertexMoveMenuPress(openmodalName);
    }
    if (ImGui::MenuItem("Scale...")) {
        VertexScaleMenuPress(openmodalName);
    }
    if (ImGui::MenuItem("Mirror / Project ...")) {
        VertexMirrorProjectMenuPress();
    }
    if (ImGui::MenuItem("Rotate...")) {
        VertexRotateMenuPress();
    }
    if (ImGui::MenuItem("Add new...")) {
        VertexAddNewMenuPress(openmodalName);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select all vertex")) {
        interfGeom->SelectAllVertex();
    }
    if (ImGui::MenuItem("Unselect all vertex")) {
        interfGeom->UnselectAllVertex();
    }
    if (ImGui::MenuItem("Select coplanar vertex (visible on screen)")) {
        VertexCoplanarMenuPress(openmodalName);
    }
    if (ImGui::MenuItem("Select isolated vertex")) {
        interfGeom->SelectIsolatedVertices();
    }
}

static void ShowMenuView(std::string& openmodalName) {
    if (ImGui::BeginMenu("Structure")) {
        if (ImGui::MenuItem("New structure...")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->AddStruct();
        }
        if (ImGui::MenuItem("Delete structure...")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->DeleteStruct();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Show All", "Ctrl+F1")) {
            interfGeom->viewStruct = -1; // -1 will show all, number of structures is interfGeom->GetNbStructure()
        }
        if (ImGui::MenuItem("Show Previous", "Ctrl+F11")) {
            if (interfGeom->viewStruct == -1) interfGeom->viewStruct = interfGeom->GetNbStructure() - 1;
            else
                interfGeom->viewStruct = (int)Previous(interfGeom->viewStruct, interfGeom->GetNbStructure());
            interfGeom->UnselectAll();
        }
        if (ImGui::MenuItem("Show Next", "Ctrl+F12")) {
            interfGeom->viewStruct = (int)Next(interfGeom->viewStruct, interfGeom->GetNbStructure());
            interfGeom->UnselectAll();
        }
        ImGui::Separator();
        // Procedural list of memorized structures
        for (int i = 0; i < interfGeom->GetNbStructure(); i++) {
            if ( ImGui::MenuItem("Show #" + std::to_string(i+1) + " (" + interfGeom->GetStructureName(i) + ")", i+2 <=12 ? "Ctrl + F" + std::to_string(i + 2) : "")) {
                interfGeom->viewStruct = i;
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(ICON_FA_TH_LARGE "  Full Screen")) {
        LockWrapper myLock(mApp->imguiRenderLock);
        if (mApp->Get_m_bWindowed()) {
            mApp->ToggleFullscreen(); // incorrect behaviour even in legacy GUI
            mApp->PlaceComponents();
        }
        else {
            mApp->Resize(1024, 800, true);
        }
    }
    ImGui::Separator();

    if (ImGui::BeginMenu("Memorize view to")) {
        if (ImGui::MenuItem("Add new...", "CTLR+Q")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->AddView();
        }
        std::vector<std::string> shortcuts = { "F1", "F2", "F3", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12" };
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name, i < shortcuts.size() ? "Alt + " + shortcuts[i] : "")) { //mApp->views[i].names - names are empty, also in legacy GUI
                mApp->OverWriteView(i);
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    if (ImGui::BeginMenu("Select memorized", mApp->views.size()==0)) {
        // Procedural list of memorized views
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name)) {
                mApp->SelectView(i);
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Clear memorized")) {
        if (ImGui::MenuItem("Clear All")) {
            openmodalName = "Clear all views?";
                showPopup = true;
        }
        // Procedural list of memorized views
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name)) {
                mApp->ClearView(i);
            }
        }
        ImGui::EndMenu();
    }
}

void ClearAllViewsPopup() {
    float txtW = ImGui::CalcTextSize(" ").x;
    auto io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(txtW * 50, 0));
    if (ImGui::BeginPopupModal("Clear all views?")) {
        ImGui::Text("Are you sure you want to clear all remembered views?");
        if (ImGui::Button("Yes", ImVec2(txtW * 12, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->ClearAllViews();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(txtW * 12, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}

static void ShowMenuTest(std::string& openmodalName) {
    if (ImGui::MenuItem("Pipe (L/R=0.0001)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(0.0001, 0); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(0.0001, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(0.0001, 0);
        }
    }
    if (ImGui::MenuItem("Pipe (L/R=1)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1.0, 0); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1.0, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(1.0, 0);
        }
    }
    if (ImGui::MenuItem("Pipe (L/R=10)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10.0, 0); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10.0, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else
        {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(10.0, 0);
        }
    }
    if (ImGui::MenuItem("Pipe (L/R=100)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(100.0, 0); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(100.0, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else
        {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(100.0, 0);
        }
    }
    if (ImGui::MenuItem("Pipe (L/R=1000)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1000.0, 0); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1000.0, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(1000.0, 0);
        }
    }
    if (ImGui::MenuItem("Pipe (L/R=10000)")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10000.0, 0); };
            auto N = []() -> void {  LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10000.0, 0); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(10000.0, 0);
        }
    }
    //Quick test pipe
    ImGui::Separator();
    if (ImGui::MenuItem("Quick Pipe", "ALT+Q")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave(); LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(5, 5); };
            auto N = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(5, 5); };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->BuildPipe(5,5);
        }
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Triangulate Geometry")) {
        if (mApp->changedSinceSave) {
            auto Y = []() -> void { DoSave();
                LockWrapper myLock(mApp->imguiRenderLock);
                auto prg = GLProgress_GUI("Triangulating", "Triangulating");
                prg.SetVisible(true);
                GeometryTools::PolygonsToTriangles(mApp->worker.GetGeometry(), prg);
                mApp->worker.MarkToReload();
            };
            auto N = []() -> void {
                LockWrapper myLock(mApp->imguiRenderLock);
                auto prg = GLProgress_GUI("Triangulating", "Triangulating");
                prg.SetVisible(true);
                GeometryTools::PolygonsToTriangles(mApp->worker.GetGeometry(), prg);
                mApp->worker.MarkToReload();
            };
            mApp->imWnd->popup.OpenImMsgBox("File not saved", "Save current geometry?", { std::make_shared<MyButtonFunc>("Yes", Y) ,std::make_shared<MyButtonFunc>("No", N), std::make_shared<MyButtonInt>("Cancel", buttonCancel) });
        }
        else
            LockWrapper myLock(mApp->imguiRenderLock);
            auto prg = GLProgress_GUI("Triangulating", "Triangulating");
            prg.SetVisible(true);
            GeometryTools::PolygonsToTriangles(mApp->worker.GetGeometry(), prg);
            mApp->worker.MarkToReload();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("ImGui Menu")) {
        mApp->imWnd->ToggleMainMenu();
    }
    if (ImGui::MenuItem("ImGui Test Suite")) {
        mApp->imWnd->ToggleMainHub();
    }
}

#ifdef MOLFLOW // TODO polyporphysm
static void ShowMenuTime() {
    if (ImGui::MenuItem("Time settings...", "ALT+I")) {
        if (!mApp->timeSettings) mApp->timeSettings = new TimeSettings(&mApp->worker);
        mApp->timeSettings->SetVisible(true);
    }
    if (ImGui::MenuItem("Edit moments...")) {
        LockWrapper myLock(mApp->imguiRenderLock);
        if (!mApp->momentsEditor || !mApp->momentsEditor->IsVisible()) {
            SAFE_DELETE(mApp->momentsEditor);
            mApp->momentsEditor = new MomentsEditor(&mApp->worker);
            mApp->momentsEditor->Refresh();
            mApp->momentsEditor->SetVisible(true);
        }
    }
    if (ImGui::MenuItem("Edit parameters...")) {
        if (!mApp->parameterEditor) mApp->parameterEditor = new ParameterEditor(&mApp->worker);
        mApp->parameterEditor->SetVisible(true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Timewise plotter")) {
        if (!mApp->timewisePlotter) mApp->timewisePlotter = new TimewisePlotter();
        mApp->timewisePlotter->Display(&mApp->worker);
    }
    if (ImGui::MenuItem("Pressure evolution")) {
        if (!mApp->pressureEvolution) mApp->pressureEvolution = new PressureEvolution(&mApp->worker);
        mApp->pressureEvolution->SetVisible(true);
    }
}

#endif // MOLFLOW

static void ShowMenuAbout() {
    if (ImGui::MenuItem("License")) {
        mApp->imWnd->show_window_license = true;
    } //TODO
    if (ImGui::MenuItem("Check for updates...")) {
        if (!mApp->manualUpdate) {
            if (!mApp->updateLogWindow) {
                mApp->updateLogWindow = new UpdateLogWindow(mApp);
            }
            mApp->manualUpdate = new ManualUpdateCheckDialog(appName, appVersionName, mApp->appUpdater, mApp->updateLogWindow, mApp->updateFoundDialog);
        }
        mApp->manualUpdate->Refresh();
        mApp->manualUpdate->SetVisible(true);
    }
    if (ImGui::MenuItem("ImGUI")) {
        if (!mApp->imWnd->show_main_hub) {
            mApp->imWnd->ToggleMainHub();
        }
    }
}

void DeleteFacetPopup() {
    float txtW = ImGui::CalcTextSize(" ").x;
    auto io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(txtW*50,0));
    if (ImGui::BeginPopupModal("Delete selected facets?", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("Yes", ImVec2(txtW*12,0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
            LockWrapper myLock(mApp->imguiRenderLock);
            if (mApp->AskToReset()) {
                
                auto selectedFacets = interfGeom->GetSelectedFacets();
                if (mApp->worker.IsRunning()) mApp->worker.Stop_Public();
                interfGeom->RemoveFacets(selectedFacets);
                //worker.CalcTotalOutgassing();
                //interfGeom->CheckIsolatedVertex();
                mApp->UpdateModelParams();
                mApp->RefreshPlotterCombos();
                //UpdatePlotters();
                if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
                if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
                // Send to sub process
                mApp->worker.MarkToReload();
            }
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(txtW * 12, 0)) || io.KeysDown[ImGui::keyEsc]) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }
}

void ShowFacetPopups(std::string& openmodalName) {
    DeleteFacetPopup();
    TriangulatePopup();
    ExplodePupup(openmodalName);
}
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowAppMainMenuBar()
// - ShowMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the
// ImGuiWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of
// the main viewport + call BeginMenuBar() into it.
void ShowAppMainMenuBar() {
    if(!interfGeom)
        interfGeom = mApp->worker.GetGeometry();
    // TODO: Try shortcuts
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_Q)){
        ImGui::OpenPopup("testmod");
    }


    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 0.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y + 0.0f));
    static std::string openmodalName = "";
    static bool askForSave = false;
    if (ImGui::BeginMainMenuBar()) {
        ImGui::AlignTextToFramePadding();
        if (ImGui::BeginMenu(ICON_FA_FILE_ARCHIVE "  File")) {
            ImMenu::ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_MOUSE_POINTER "  Selection")) {
            ImMenu::ShowMenuSelection(openmodalName);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_TOOLS "  Tools")) {
            ImMenu::ShowMenuTools();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Facet")) {
            ImMenu::ShowMenuFacet(openmodalName);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Vertex")) {
            ImMenu::ShowMenuVertex(openmodalName);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImMenu::ShowMenuView(openmodalName);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Test")) {
            ImMenu::ShowMenuTest(openmodalName);
            ImGui::EndMenu();
        }
        #ifdef MOLFLOW // TODO Polymorphism
        if (ImGui::BeginMenu("Time")) {
            ImMenu::ShowMenuTime();
            ImGui::EndMenu();
        }
        #endif //MOLFLOW
        if (ImGui::BeginMenu("About")) {
            ImMenu::ShowMenuAbout();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (!ImMenu::modalTitle.empty()) {
        openmodalName = ImMenu::modalTitle;
    }
    if(!openmodalName.empty()){
        ImGui::OpenPopup(openmodalName.c_str());
        openmodalName = "";
    }
    if (ImMenu::showPopup) {
        ImGui::CaptureKeyboardFromApp(ImMenu::showPopup); // TODO: consolidate popup definitions
        ImMenu::ShowSelectionModals();
        ImMenu::ShowFacetPopups(openmodalName);
        ImMenu::RemoveVertexPopup();
        ImMenu::SelectCoplanarVerticesPopup(openmodalName);
        ImMenu::ClearAllViewsPopup();
        ImMenu::ErrorPopup();
        ImMenu::ShowPopup();
    }

    ImGui::PopStyleVar(2);
}