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

#include "SmartSelection.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "SelectFacetByResult.h"
#include "FormulaEditor.h"
#include "ConvergencePlotter.h"
#include "../../molflow/src/Interface/TexturePlotter.h"

#include "Worker.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"
#include "ImguiWindow.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "../Helper/StringHelper.h"

#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"

#if defined(MOLFLOW)

#include "../../src/MolFlow.h"

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
static size_t selectionId = -1;

bool AskToSave() {
    if (!mApp->changedSinceSave)
        return true;
    LockWrapper LockWrapper(mApp->imguiRenderLock);
    return mApp->AskToSave();
}/*
    ImGuiIO& io = ImGui::GetIO();
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    bool returnVal = false;
    if (ImGui::BeginPopupModal("File not saved", NULL, ImGuiWindowFlags_AlwaysAutoResize)){
        ImGui::Text("Save current geometry first?");

        if (ImGui::Button("Yes") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            *saveModal = false;
            std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
            if (!fn.empty()) {
                auto prg = GLProgress_GUI("Saving file...", "Please wait"); //replace with ImGui progress
                prg.SetVisible(true);
                //GLWindowManager::Repaint();
                try {
                    mApp->worker.SaveGeometry(fn, prg);
                    mApp->changedSinceSave = false;
                    mApp->UpdateTitle();
                    mApp->AddRecent(fn);
                }
                catch (const std::exception& e) {
                    std::string errMsg = ("%s\nFile:%s", e.what(), fn.c_str());
                    ImguiPopup::Popup(errMsg, "Error");
                    mApp->RemoveRecent(fn.c_str());
                }
                returnVal = true;
            }
            else
                returnVal = false;
        }
        ImGui::SameLine();
        if (ImGui::Button("No")) {
            *saveModal = false;
            returnVal = true;
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel") || io.KeysDown[ImGui::keyEsc]) {
            *saveModal = false;
            returnVal = false;
        }

        if(!saveModal) {
            ImGui::CloseCurrentPopup();
            showPopup = false;
        }
        ImGui::EndPopup();
    }

    return false;
}*/

void NewEmptyGeometryButtonPress() {
    //ImGui::OpenPopup("File not saved");
    if (AskToSave()) {
        if (mApp->worker.IsRunning())
            mApp->worker.Stop_Public();
        {
            LockWrapper LockWrapper(mApp->imguiRenderLock);
            mApp->EmptyGeometry();
        }
    }
}

void LoadFileButtonPress() {
    if (AskToSave()) {
        if (mApp->worker.IsRunning())
            mApp->worker.Stop_Public();
        {
            LockWrapper LockWrapper(mApp->imguiRenderLock);
            mApp->LoadFile("");
        }
    }
}

void InsertGeometryButtonPress(bool newStr) {
    InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
    if (interfGeom->IsLoaded()) {
        if (mApp->worker.IsRunning())
            mApp->worker.Stop_Public();
        {
            LockWrapper LockWrapper(mApp->imguiRenderLock);
            mApp->InsertGeometry(newStr, "");
        }
    }
    else ImguiPopup::Popup("No geometry loaded.","No geometry");
}

void SaveButtonPress() {
    InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
    if (interfGeom->IsLoaded()) {
        LockWrapper LockWrapper(mApp->imguiRenderLock);
        mApp->SaveFile();
    }
    else ImguiPopup::Popup("No geometry loaded.", "No geometry");
}

void SaveAsButtonPress() {
    InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
    if (interfGeom->IsLoaded()) {
        LockWrapper LockWrapper(mApp->imguiRenderLock);
        mApp->SaveFileAs();
    }
    else ImguiPopup::Popup("No geometry loaded.", "No geometry");
}

void ExportSelectedFacetsButtonPress() {
    LockWrapper LockWrapper(mApp->imguiRenderLock);
    mApp->ExportSelection();
}

void ExportSelectedProfilesButtonPress() {
    LockWrapper LockWrapper(mApp->imguiRenderLock);
    mApp->ExportProfiles();
}

#ifdef MOLFLOW
void ShowSubmenuExportTextures(bool coord=false) {
    if(ImGui::MenuItem("Cell Area (cm^2)")) {} // imgui does not support superscript
    if(ImGui::MenuItem("# of MC Hits")) {}
    if(ImGui::MenuItem("Impingement rate (1/s/m^2)")) {}
    if(ImGui::MenuItem("Particle density (1/m^3)")) {}
    if(ImGui::MenuItem("Gas density (kg/m^3)")) {}
    if(ImGui::MenuItem("Pressure (mbar)")) {}
    if(ImGui::MenuItem("Avg. Velocity (m/s)")) {}
    if(ImGui::MenuItem("Velocity vector (m/s)")) {}
    if(ImGui::MenuItem("# of velocity vectors")) {}
}

void ImportDesorptionFromSYNFileButtonPress() {
    // TODO
}
#endif // MOLFLOW

static void ShowMenuFile(bool &askToSave) {
    if(ImGui::MenuItem(ICON_FA_PLUS "  New, empty geometry")){
        NewEmptyGeometryButtonPress();
    }
    if(ImGui::MenuItem(ICON_FA_FILE_IMPORT "  Load", "Ctrl+O")){
        LoadFileButtonPress();
    }
    if(ImGui::MenuItem(ICON_FA_ARROW_CIRCLE_LEFT "  Load recent")){
        // TODO
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Insert geometry")) {
        if(ImGui::MenuItem("To current structure")){
            InsertGeometryButtonPress(false);
        }
        if(ImGui::MenuItem("To new structure")){
            InsertGeometryButtonPress(true);
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();
    if(ImGui::MenuItem(ICON_FA_SAVE "  Save", "Ctrl+S")){
        if (mApp->worker.GetGeometry()->IsLoaded())
            SaveButtonPress();
    }
    if(ImGui::MenuItem(ICON_FA_SAVE "  Save as")){
        if (mApp->worker.GetGeometry()->IsLoaded())
            SaveAsButtonPress();
    }
    ImGui::Separator();

    if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "  Export selected facets")) {
        ExportSelectedFacetsButtonPress();
    }

    if (ImGui::MenuItem(ICON_FA_FILE_EXPORT "  Export selected profiles")) {
        ExportSelectedProfilesButtonPress();
    }

    // TODO: Molflow only entries
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
        ImportDesorptionFromSYNFileButtonPress();
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
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        //ImGui::SetItemDefaultFocus();
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
        char selectionName[128];

        std::stringstream tmp;
        tmp << "Selection #" << (mApp->selections.size() + 1);
        strcpy(selectionName, tmp.str().c_str());
        ImGui::InputText("Selection name", selectionName, 128);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0)) || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyNumEnter]) {
            if (strcmp(selectionName, "") != 0)
                mApp->AddSelection(selectionName);
            ImGui::CloseCurrentPopup();
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
        ImGui::Text(modalMsg);
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            try {
                mApp->selections.erase(mApp->selections.begin() + (selectionId ));
                ImGui::CloseCurrentPopup();
            }
            catch (std::exception e) {
                ImGui::EndPopup();
                throw e;
            }
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
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

#if defined(MOLFLOW)
    if (ImGui::MenuItem("Select Sticking", "")) {
        // TODO: Different for Synrad?
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (!interfGeom->GetFacet(i)->sh.stickingParam.empty() ||
                (interfGeom->GetFacet(i)->sh.sticking != 0.0 && !interfGeom->GetFacet(i)->IsTXTLinkFacet()))
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
#endif

    if (ImGui::MenuItem("Select Transparent", "")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)
            if (
#if defined(MOLFLOW)
!interfGeom->GetFacet(i)->sh.opacityParam.empty() ||
#endif
(interfGeom->GetFacet(i)->sh.opacity != 1.0 && interfGeom->GetFacet(i)->sh.opacity != 2.0))
                interfGeom->SelectFacet(i);
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
    //if(ImGui::MenuItem(nullptr) {} // Separator
    //if(ImGui::MenuItem("Load selection",MENU_FACET_LOADSEL) {}
    //if(ImGui::MenuItem("Save selection",MENU_FACET_SAVESEL) {}
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
                selectionId = i;
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

void FormulaEditorButtonPress() {
    InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
    if (!interfGeom->IsLoaded()) {
        ImguiPopup::Popup("No geometry loaded.", "No geometry");
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
void ConvergencePlotterButtonPress() {
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
void TexturePlotterButtonPress() {
    if (!mApp->texturePlotter) mApp->texturePlotter = new TexturePlotter();
    mApp->texturePlotter->Display(&mApp->worker);
}
void ProfilePlotterButtonPress() {}
void HistogramPlotterButtonPress() {}
void TextureScalingButtonPress() {}
void ParticleLoggerButtonPress() {}

void TakeScreenshotButtonPress() {}
void MovingPartsButtonPress() {}

static void ShowMenuTools() {
    if (ImGui::MenuItem("Formula editor", "ALT+F")) {
        FormulaEditorButtonPress(); // TODO: replace with Toggle ImGui Formula Editor
    }
    if (ImGui::MenuItem("Convergence Plotter ...", "ALT+C")) {
        ConvergencePlotterButtonPress();  // TODO: replace with Toggle ImGui Convergence Plotter
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Texture Plotter ...", "ALT+T")) {
        TexturePlotterButtonPress();
    }
    if (ImGui::MenuItem("Profile Plotter ...", "ALT+P")) {}
#if defined(MOLFLOW)
    if (ImGui::MenuItem("Histogram Plotter...")) {}
#endif
    ImGui::Separator();
    if (ImGui::MenuItem("Texture scaling...", "CTRL+D")) {}
    if (ImGui::MenuItem("Particle logger...")) {}
    //if (ImGui::MenuItem("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER)){}
    if (ImGui::MenuItem("Global Settings ...")) {
        mApp->imWnd->show_global_settings = true;
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Take screenshot", "CTRL+R")) {}

    // TODO: Extract Molflow only entries
    ImGui::Separator();
    if (ImGui::MenuItem("Moving parts...")) {}
}

void DeleteButtonPress() {}
void SwapNormalButtonPress() {}
void ShiftIndicesButtonPress() {}
void FacetCoordinatesButtonPress() {}

void ScaleButtonPress() {}
void MirrorProjectButtonPress() {}
void RotateButtonPress() {}
void AlignToButtonPress() {}
void ExtrudeButtonPress() {}
void SplitButtonPress() {}
void CreateShapeButtonPress() {}
// Create two facets
static void ShowMenuFacet() {
    if (ImGui::MenuItem("Delete", "CTRL+DEL")) {}
    if (ImGui::MenuItem("Swap normal", "CTRL+N")) {}
    if (ImGui::MenuItem("Shift indices", "CTRL+H")) {}
    if (ImGui::MenuItem("Facet coordinates ...")) {}
    if (ImGui::MenuItem("Move ...")) {
        mApp->imWnd->show_facet_move = true;
    }
    if (ImGui::MenuItem("Scale ...")) {}
    if (ImGui::MenuItem("Mirror / Project ...")) {}
    if (ImGui::MenuItem("Rotate ...")) {}
    if (ImGui::MenuItem("Align to ...")) {}
    if (ImGui::MenuItem("Extrude ...")) {}
    if (ImGui::MenuItem("Split ...")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Create shape...")) {}
    if (ImGui::BeginMenu("Create two facets' ...")) {
        if (ImGui::BeginMenu("Create two facets' ...")) {
            if (ImGui::MenuItem("Auto (non-zero)")) {}
            if (ImGui::MenuItem("First - Second")) {}
            if (ImGui::MenuItem("Second - First")) {}
            ImGui::EndMenu();
        }
        if (ImGui::MenuItem("Union")) {}
        if (ImGui::MenuItem("Intersection")) {}
        if (ImGui::MenuItem("XOR")) {}
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Transition between 2")) {}
    if (ImGui::MenuItem("Build intersection...")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Collapse ...")) {}
    if (ImGui::MenuItem("Explode")) {}
    if (ImGui::MenuItem("Revert flipped normals (old geometries)")) {}
    if (ImGui::MenuItem("Triangulate")) {}
}

static void ShowMenuVertex() {
    if (ImGui::BeginMenu("Create Facet from Selected")) {
        if (ImGui::MenuItem("Convex Hull", "ALT+V")) {}
        if (ImGui::MenuItem("Keep selection order")) {}
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Clear isolated")) {}
    if (ImGui::MenuItem("Remove selected")) {}
    if (ImGui::MenuItem("Vertex coordinates...")) {}
    if (ImGui::MenuItem("Move...")) {}
    if (ImGui::MenuItem("Scale...")) {}
    if (ImGui::MenuItem("Mirror / Project ...")) {}
    if (ImGui::MenuItem("Rotate...")) {}
    if (ImGui::MenuItem("Add new...")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Select all vertex")) {}
    if (ImGui::MenuItem("Unselect all vertex")) {}
    if (ImGui::MenuItem("Select coplanar vertex (visible on screen)")) {}
    if (ImGui::MenuItem("Select isolated vertex")) {}
}

static void ShowMenuView() {
    if (ImGui::BeginMenu("Structure")) {
        //UpdateStructMenu();
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(ICON_FA_TH_LARGE "  Full Screen")) {}
    ImGui::Separator();

    if (ImGui::BeginMenu("Memorize view to")) {
        if (ImGui::MenuItem("Add new...", "CTLR+Q")) {}
        ImGui::EndMenu();
    }
    ImGui::Separator();

    if (ImGui::MenuItem("Select memorized")) {}
    if (ImGui::BeginMenu("Clear memorized")) {
        if (ImGui::MenuItem("Clear All")) {}
        ImGui::EndMenu();
    }
}

static void ShowMenuTest() {
    if (ImGui::MenuItem("Pipe (L/R=0.0001)")) {}
    if (ImGui::MenuItem("Pipe (L/R=1)")) {}
    if (ImGui::MenuItem("Pipe (L/R=10)")) {}
    if (ImGui::MenuItem("Pipe (L/R=100)")) {}
    if (ImGui::MenuItem("Pipe (L/R=1000)")) {}
    if (ImGui::MenuItem("Pipe (L/R=10000)")) {}
    //Quick test pipe
    ImGui::Separator();
    if (ImGui::MenuItem("Quick Pipe", "ALT+Q")) {}

    ImGui::Separator();
    if (ImGui::MenuItem("Triangulate Geometry")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("ImGui Menu")) {
        mApp->imWnd->ToggleMainMenu();
    }
    if (ImGui::MenuItem("ImGui Test Suite")) {
        mApp->imWnd->ToggleMainHub();
    }
}

// TODO: Only in Molflow
static void ShowMenuTime() {
    if (ImGui::MenuItem("Time settings...", "ALT+I")) {}
    if (ImGui::MenuItem("Edit moments...")) {}
    if (ImGui::MenuItem("Edit parameters...")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Timewise plotter")) {}
    if (ImGui::MenuItem("Pressure evolution")) {}
}

static void ShowMenuAbout() {
    if (ImGui::MenuItem("License")) {}
    if (ImGui::MenuItem("Check for updates...")) {}
    if (ImGui::MenuItem("ImGUI")) {}
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
    // TODO: Try shortcuts
    if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_Q)){
        ImGui::OpenPopup("testmod");
    }

    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 8.f));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y + 8.0f));
    static std::string openmodalName = "";
    static bool askForSave = false;
    if (ImGui::BeginMainMenuBar()) {
        ImGui::AlignTextToFramePadding();
        if (ImGui::BeginMenu(ICON_FA_FILE_ARCHIVE "  File")) {
            ShowMenuFile(askForSave);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_MOUSE_POINTER "  Selection")) {
            ShowMenuSelection(openmodalName);
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_TOOLS "  Tools")) {
            ShowMenuTools();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Facet")) {
            ShowMenuFacet();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Vertex")) {
            ShowMenuVertex();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ShowMenuView();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Test")) {
            ShowMenuTest();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Time")) {
            ShowMenuTime();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("About")) {
            ShowMenuAbout();
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if(!openmodalName.empty()){
        ImGui::OpenPopup(openmodalName.c_str());
        openmodalName = "";
    }
    if (showPopup) {
        ImGui::CaptureKeyboardFromApp(showPopup);
        ShowSelectionModals();
    }

    ImGui::PopStyleVar(2);
}