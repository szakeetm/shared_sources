#include "ImguiMenu.h"
#include "imgui.h"
#include <imgui_fonts/IconsFontAwesome5.h>
#include <imgui_fonts/IconsMaterialDesign.h>
#include <Helper/MathTools.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"

#include "VertexCoordinates.h"
#include "FacetCoordinates.h"
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
#include "../src/Interface/TexturePlotter.h"
#include "../src/Interface/ProfilePlotter.h"
#include "../src/Interface/TextureScaling.h"
#include "../src/Interface/Movement.h"
#include "../src/Interface/MeasureForce.h"
#include "../src/Interface/OutgassingMapWindow.h"
#include "../src/Interface/PressureEvolution.h"
#include "../src/Interface/TimewisePlotter.h"
#include "../src/Interface/TimeSettings.h"
#include "../src/Interface/MomentsEditor.h"
#include "../src/Interface/ParameterEditor.h"
#include "../src/Interface/ImportDesorption.h"

#include "../src/versionId.h"

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

void DoLoadFile() {
    if (mApp->worker.IsRunning())
        mApp->worker.Stop_Public();
    {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->LoadFile("");
    }
}

void InsertGeometryMenuPress(const bool newStr) {
    
    if (interfGeom->IsLoaded()) {
        if (mApp->worker.IsRunning())
            mApp->worker.Stop_Public();
        {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->InsertGeometry(newStr, "");
        }
    }
    else 
        mApp->imWnd->popup.Open("No geometry", "No geometry loaded.", { std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk) });
}

void SaveAsMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->SaveFileAs();
    }
    else 
        mApp->imWnd->popup.Open("No geometry", "No geometry loaded.", { std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk) });
}

void ExportSelectedFacetsMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportSelection();
}

void ExportSelectedProfilesMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportProfiles();
}

void ExportTextures(const int a, const int b) {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->ExportTextures(a, b);
}

#ifdef MOLFLOW //TODO replace with polimorphism
void ShowSubmenuExportTextures(const bool coord=false) {
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
    else
        mApp->imWnd->popup.Open("No geometry", "No geometry loaded.", { std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk) });

}
#endif // MOLFLOW

void NewGeometry() {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->worker.IsRunning())
        mApp->worker.Stop_Public();
    mApp->EmptyGeometry();
}

void DoLoadSelected(std::string file) {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->LoadFile(file);
}

void LoadMenuButtonPress() {
    ImIOWrappers::AskToSaveBeforeDoing([]() { DoLoadFile(); });
}

void QuitMenuPress() {
    auto common = []() { exit(0); };
    ImIOWrappers::AskToSaveBeforeDoing(common);
}

static void ShowMenuFile() {
    if(ImGui::MenuItem(ICON_FA_PLUS "  New, empty geometry###NewGeom")) {
        auto common = []() { NewGeometry(); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }

    if(ImGui::MenuItem(ICON_FA_FILE_IMPORT "  Load", mApp->imWnd->ctrlText+"+O")){
        LoadMenuButtonPress();
    }
    if (mApp->recentsList.empty()) {
        ImGui::BeginDisabled();
        ImGui::MenuItem("Load recent");
        ImGui::EndDisabled();
    }
    else if(ImGui::BeginMenu(ICON_FA_ARROW_CIRCLE_LEFT "  Load recent")){
        for (int i = static_cast<int>(mApp->recentsList.size()) - 1; i >= 0; i--) {
            if (ImGui::MenuItem(AbbreviateString(mApp->recentsList[i], 128))) {
                std::string selection = mApp->recentsList[i];
                auto common = [selection]() { DoLoadSelected(selection); };
                ImIOWrappers::AskToSaveBeforeDoing(common);
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
    if(ImGui::MenuItem(ICON_FA_SAVE "  Save", mApp->imWnd->ctrlText+"+S")){
        if (mApp->worker.GetGeometry()->IsLoaded())
            ImIOWrappers::DoSave();
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
        QuitMenuPress();
    }
}

void InvertSelectionMenuPress() {
    InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
        interfGeom->GetFacet(i)->selected = !interfGeom->GetFacet(i)->selected;
    interfGeom->UpdateSelection();
    mApp->UpdateFacetParams(true);
}

void UpdateSelectionShortcuts() {
    mApp->imWnd->shortcutMan.UnregisterShortcut(5);
    for (int i = 0; i < mApp->selections.size() && i+1<=9; i++) {
        std::function<void()> F = [i]() { mApp->SelectSelection(i); };
        mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_1 + i }, F, 5);
    }
}

void NewSelectionMemoryMenuPress() {
    auto F = [](std::string arg) {
        mApp->AddSelection(arg);
        UpdateSelectionShortcuts();
        };
    mApp->imWnd->input.Open("Enter selection name", "Selection name", F, "Selection #" + std::to_string(mApp->selections.size() + 1));
}

static void ShowMenuSelection() {
    static SelectDialog *selectDialog = nullptr;
    static SelectTextureType *selectTextureType = nullptr;

    Worker &worker = mApp->worker;
    InterfaceGeometry *interfGeom = worker.GetGeometry();

    if (ImGui::MenuItem("Smart Select facets...", "Alt+S")) {
        mApp->imWnd->smartSelect.Show();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select All Facets", mApp->imWnd->ctrlText+"+A")) {
        interfGeom->SelectAll();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select by Facet Number...", "Alt+N")) {
        mApp->imWnd->selByNum.Show();
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
        mApp->imWnd->selByTex.Show();
        //if (!selectTextureType) selectTextureType = new SelectTextureType(&worker);
        //selectTextureType->SetVisible(true);
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
        auto F = [](std::string arg) {
            if (!Util::getNumber(&mApp->largeAreaThreshold, arg)) {
                mApp->imWnd->popup.Open("Error", "Incorrect value", {std::make_shared<ImIOWrappers::ImButtonInt>("Ok",ImIOWrappers::buttonOk, ImGuiKey_Enter)});
            } else {
                InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
                interfGeom->UnselectAll();
                for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                    if (interfGeom->GetFacet(i)->facetHitCache.nbMCHit == 0 &&
                        interfGeom->GetFacet(i)->sh.area >= mApp->largeAreaThreshold)
                        interfGeom->SelectFacet(i);
                interfGeom->UpdateSelection();
                mApp->UpdateFacetParams(true);
            }
        };
        mApp->imWnd->input.Open("Select large facets without hits", u8"Min.area (cm\u00b2)", F, fmt::format("{:.3g}",mApp->largeAreaThreshold));
    }
    if (ImGui::MenuItem("Select by facet result...")) {
        mApp->imWnd->selFacetByResult.Show();
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
        auto F = [](std::string arg) {
            if (!Util::getNumber(&mApp->planarityThreshold, arg)) {
                mApp->imWnd->popup.Open("Error", "Incorrect value", { 
                    std::make_shared<ImIOWrappers::ImButtonInt>("Ok",ImIOWrappers::buttonOk,ImGuiKey_Enter) 
                    });
            }
            else {
                InterfaceGeometry* interfGeom = mApp->worker.GetGeometry();
                interfGeom->UnselectAll();
                std::vector<size_t> nonPlanarFacetids = interfGeom->GetNonPlanarFacetIds(mApp->planarityThreshold);
                for (const auto& i : nonPlanarFacetids)
                    interfGeom->SelectFacet(i);
                interfGeom->UpdateSelection();
                mApp->UpdateFacetParams(true);
            }
        };
        mApp->imWnd->input.Open("Select non planar facets", "Planarity larger than", F, fmt::format("{:.3g}", mApp->planarityThreshold));
    }
    if (ImGui::MenuItem("Select non simple facets")) {
        interfGeom->UnselectAll();
        for (int i = 0; i < interfGeom->GetNbFacet(); i++)

            if (interfGeom->GetFacet(i)->nonSimple)
                interfGeom->SelectFacet(i);
        interfGeom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    if (ImGui::MenuItem("Invert selection", mApp->imWnd->ctrlText+"+I")) {
        InvertSelectionMenuPress();
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Memorize selection to")) {
        if (ImGui::MenuItem("Add new...", mApp->imWnd->ctrlText+"+W")) {
            NewSelectionMemoryMenuPress();
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                auto F = [i, interfGeom](std::string str) {
                    mApp->selections[i].facetIds = interfGeom->GetSelectedFacets();
                    mApp->selections[i].name=str;
                    };
                mApp->imWnd->input.Open("Overwrite "+ mApp->selections[i].name + "?", "Enter new name", F, mApp->selections[i].name);
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Select memorized", !mApp->selections.empty())) {
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (i <= 8) {
                char shortcut[32];
                sprintf(shortcut, "Alt+%llu", 1 + i);
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
        if (ImGui::MenuItem("Select previous", "Alt+F11")) {
            mApp->SelectSelection(Previous(mApp->idSelection, mApp->selections.size()));
        }
        if (ImGui::MenuItem("Select next", "Alt+F12")) {
            mApp->SelectSelection(Next(mApp->idSelection, mApp->selections.size()));
        }
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Clear memorized")) {
        if (ImGui::MenuItem("Clear All")) {
            auto Y = []() -> void { mApp->ClearAllSelections(); UpdateSelectionShortcuts(); };
            mApp->imWnd->popup.Open("Clear All?", "Clear all memorized selections?.", {
                std::make_shared<ImIOWrappers::ImButtonFunc>("Yes", Y, ImGuiKey_Enter),
                std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape) 
                });
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                auto Y = [](int i) -> void { mApp->selections.erase(mApp->selections.begin() + (i)); UpdateSelectionShortcuts(); };
                mApp->imWnd->popup.Open("Clear memorized selection?", ("Are you sure you wish to forget selection " + mApp->selections[i].name), {
                    std::make_shared<ImIOWrappers::ImButtonFuncInt>("Yes", Y, (int)i, ImGuiKey_Enter),
                    std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape) 
                    });
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
}

void FormulaEditorMenuPress() {
    
    if (!interfGeom->IsLoaded()) {
        ImIOWrappers::InfoPopup("No geometry", "No geometry loaded.");
        return;
    }
    mApp->imWnd->formulaEdit.Show();
    /*
    else if (!mApp->formulaEditor || !mApp->formulaEditor->IsVisible()) {
        SAFE_DELETE(mApp->formulaEditor);
        mApp->formulaEditor = new FormulaEditor(&mApp->worker, mApp->appFormulas);
        mApp->formulaEditor->Refresh();
        // Load values on init
        mApp->appFormulas->EvaluateFormulas(mApp->worker.globalStatCache.globalHits.nbDesorbed);
        mApp->formulaEditor->UpdateValues();
        // ---
        mApp->formulaEditor->SetVisible(true);
    }*/
}
void ConvergencePlotterMenuPress() {
    mApp->imWnd->convPlot.Show();
    /*if (!mApp->convergencePlotter)
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
       */
}
#ifdef MOLFLOW //TODO replace with polimorphism
void TexturePlotterMenuPress() {
    mApp->imWnd->textPlot.Show();
    /*
    if (!mApp->texturePlotter) mApp->texturePlotter = new TexturePlotter();
    mApp->texturePlotter->Display(&mApp->worker);*/
}

void ProfilePlotterMenuPress() {
    mApp->imWnd->profPlot.Show();
    //if (!mApp->profilePlotter) mApp->profilePlotter = new ProfilePlotter(&mApp->worker);
    //mApp->profilePlotter->Display(&mApp->worker);
}
#endif //MOLFLOW

void HistogramPlotterMenuPress() {
    mApp->imWnd->histPlot.Show();
    /*
    if (!mApp->histogramPlotter || !mApp->histogramPlotter->IsVisible()) {
        SAFE_DELETE(mApp->histogramPlotter);
        mApp->histogramPlotter = new HistogramPlotter(&mApp->worker);
    }
    mApp->histogramPlotter->SetVisible(true);
    */
}
#if defined(MOLFLOW)
void TextureScalingMenuPress() {
    mApp->imWnd->textScale.Show();
    /*if (!mApp->textureScaling || !mApp->textureScaling->IsVisible()) {
        SAFE_DELETE(mApp->textureScaling);
        mApp->textureScaling = new TextureScaling();
        mApp->textureScaling->Display(&mApp->worker, mApp->viewers);
    }*/
}
#endif
void ParticleLoggerMenuPress() {
    mApp->imWnd->partLog.Show();
    /*if (!mApp->particleLogger || !mApp->particleLogger->IsVisible()) {
        SAFE_DELETE(mApp->particleLogger);
        
        mApp->particleLogger = new ParticleLogger(interfGeom, &mApp->worker);
    }
    LockWrapper lockWrapper(mApp->imguiRenderLock);
    mApp->particleLogger->UpdateStatus();
    mApp->particleLogger->SetVisible(true);*/
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
    mApp->viewers[mApp->curViewer]->GetBounds(&x, &y, &width, &height);

    int leftMargin = 4; //Left bewel
    int rightMargin = 0;
    int topMargin = 0;
    int bottomMargin = 28; //Toolbar

    mApp->viewers[mApp->curViewer]->RequestScreenshot(tmp_ss.str(), leftMargin, topMargin,
        width - leftMargin - rightMargin,
        height - topMargin - bottomMargin);
}

#ifdef MOLFLOW //TODO replace with polimorphism
void MovingPartsMenuPress() {
    mApp->imWnd->movPart.Show();
    /*
    if (!mApp->movement) mApp->movement = new Movement(interfGeom, &mApp->worker);
    mApp->movement->Update();
    mApp->movement->SetVisible(true);*/
}
void MeasureForcesMenuPress() {
    mApp->imWnd->measForce.Show();
    /*
    if (!mApp->measureForces) mApp->measureForces = new MeasureForce(interfGeom, &mApp->worker);
    mApp->measureForces->Update();
    mApp->measureForces->SetVisible(true);
    */
}
#endif //MOLFLOW

static void ShowMenuTools() {
    if (ImGui::MenuItem(u8"\u221AFormula editor###Formula editor", "Alt+F")) {
        FormulaEditorMenuPress();
    }
    if (ImGui::MenuItem("Convergence Plotter ...", "Alt+C")) {
        ConvergencePlotterMenuPress();  // TODO: replace with Toggle ImGui Convergence Plotter
    }
    ImGui::Separator();

#if defined(MOLFLOW)
    if (ImGui::MenuItem("Texture Plotter ...", "Alt+T")) {
        TexturePlotterMenuPress();
    }
    if (ImGui::MenuItem("Profile Plotter ...", "Alt+P")) {
        ProfilePlotterMenuPress();
    }
#endif
    ImGui::Separator();
    if (ImGui::MenuItem("Histogram Plotter...")) {
        HistogramPlotterMenuPress();
    }
    if (ImGui::MenuItem("Texture scaling...", mApp->imWnd->ctrlText+"+D")) {
        TextureScalingMenuPress();
    }
    if (ImGui::MenuItem("Particle logger...")) {
        ParticleLoggerMenuPress();
    }
    //if (ImGui::MenuItem("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER)){}
    if (ImGui::MenuItem("Global Settings ...")) {
        mApp->imWnd->globalSet.Show();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Take screenshot", mApp->imWnd->ctrlText+"+R")) {
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

void FacetDeleteMenuPress() {
    
    auto selectedFacets = interfGeom->GetSelectedFacets();
    if (selectedFacets.empty()) return; //Nothing selected
    auto Y = []() -> void {
        LockWrapper myLock(mApp->imguiRenderLock);
        if (mApp->AskToReset()) {
            auto selectedFacets = interfGeom->GetSelectedFacets();
            if (mApp->worker.IsRunning()) mApp->worker.Stop_Public();
            interfGeom->RemoveFacets(selectedFacets);
            //worker.CalcTotalOutgassing();
            //interfGeom->CheckIsolatedVertex();
            mApp->UpdateModelParams();
            mApp->RefreshPlotterCombos();
            mApp->UpdatePlotters();
            // Send to sub process
            mApp->worker.MarkToReload();
        }
     };
    mApp->imWnd->popup.Open("Delete selected facets?", "Delete all selected facets?", {
        std::make_shared<ImIOWrappers::ImButtonFunc>("Yes", Y, ImGuiKey_Enter),
        std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape)
        });

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
    mApp->imWnd->facCoord.UpdateFromSelection();
    LockWrapper myLock(mApp->imguiRenderLock);
    
    if (mApp->AskToReset()) {
        interfGeom->ShiftVertex();
        // Send to sub process
        mApp->worker.MarkToReload();
    }
}
void FacetCoordinatesMenuPress() {
    mApp->imWnd->facCoord.Show();
    /*
    if (!mApp->facetCoordinates) mApp->facetCoordinates = new FacetCoordinates();
    mApp->facetCoordinates->Display(&mApp->worker);
    mApp->facetCoordinates->SetVisible(true);
    */
}

void FacetScaleMenuPress() {
    if (interfGeom->IsLoaded()) {
        mApp->imWnd->facScale.Show();
        /*
        if (!mApp->scaleFacet) mApp->scaleFacet = new ScaleFacet(interfGeom, &mApp->worker);

        mApp->scaleFacet->SetVisible(true);

    */
    }
    else {
        mApp->imWnd->popup.Open("No Geometry", "", {
            std::make_shared<ImIOWrappers::ImButtonInt>("Ok", ImIOWrappers::buttonOk, ImGuiKey_Enter)
            });
    }
}
void FacetMirrorProjectMenuPress() {
    mApp->imWnd->mirrProjFacet.Show();
    /*
    if (!mApp->mirrorFacet) mApp->mirrorFacet = new MirrorFacet(interfGeom, &mApp->worker);
    mApp->mirrorFacet->SetVisible(true);
    */
}
void FacetRotateMenuPress() {
    mApp->imWnd->rotFacet.Show();
    /*
    if (!mApp->rotateFacet) mApp->rotateFacet = new RotateFacet(interfGeom, &mApp->worker);
    mApp->rotateFacet->SetVisible(true);
    */
}
void AlignToMenuPress() {
    mApp->imWnd->alignFacet.Show();
    /*
    if (!mApp->alignFacet) mApp->alignFacet = new AlignFacet(interfGeom, &mApp->worker);
    mApp->alignFacet->MemorizeSelection();
    mApp->alignFacet->SetVisible(true);
    */
}
void ExtrudeMenuPress() {
    mApp->imWnd->extrudeFacet.Show();
    /*
    if (!mApp->extrudeFacet || !mApp->extrudeFacet->IsVisible()) {
        SAFE_DELETE(mApp->extrudeFacet);
        mApp->extrudeFacet = new ExtrudeFacet(interfGeom, &mApp->worker);
    }
    mApp->extrudeFacet->SetVisible(true);
    */
}
void SplitMenuPress() {
    mApp->imWnd->splitFac.Show();
    /*
    if (!mApp->splitFacet || !mApp->splitFacet->IsVisible()) {
        SAFE_DELETE(mApp->splitFacet);
        mApp->splitFacet = new SplitFacet(interfGeom, &mApp->worker);
        mApp->splitFacet->SetVisible(true);
    }
    */
}
void CreateShapeMenuPress() {
    mApp->imWnd->createShape.Show();
    /*
    if (!mApp->createShape) mApp->createShape = new CreateShape(interfGeom, &mApp->worker);
    mApp->createShape->SetVisible(true);
    */
}

void TransitionBetween2MenuPress() {
    
    if (interfGeom->GetNbSelectedFacets() != 2) {
        mApp->imWnd->popup.Open("Error", "Select Exactly 2 facets", {
            std::make_shared<ImIOWrappers::ImButtonInt>("Ok", ImIOWrappers::buttonOk, ImGuiKey_Enter)
            });
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
    mApp->imWnd->buildIntersect.Show();
    /*
    if (!mApp->buildIntersection || !mApp->buildIntersection->IsVisible()) {
        SAFE_DELETE(mApp->buildIntersection);
        mApp->buildIntersection = new BuildIntersection(interfGeom, &mApp->worker);
        mApp->buildIntersection->SetVisible(true);
    }
    */
}
void CollapseMenuPress() {
    if (interfGeom->IsLoaded()) {
        mApp->imWnd->collapseSettings.Show();
        //mApp->DisplayCollapseDialog();
    }
    else mApp->imWnd->popup.Open("Error", "No Geometry", {
        std::make_shared<ImIOWrappers::ImButtonInt>("Ok", ImIOWrappers::buttonOk, ImGuiKey_Enter)
        });
}

void ExplodeMenuPress() {
    mApp->imWnd->expFac.Show();
}

void RevertMenuPress() {
    
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        interfGeom->RevertFlippedNormals();
        // Send to sub process
        mApp->worker.MarkToReload();
    }
}

void TriangulateMenuPress() {
    
    auto selectedFacets = interfGeom->GetSelectedFacets();
    if (selectedFacets.empty()) return;
    auto Y = []() -> void {
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
    };
    mApp->imWnd->popup.Open("Triangulate entire geometry?", "This operation cannot be undone!", {
        std::make_shared<ImIOWrappers::ImButtonFunc>("Yes",Y,ImGuiKey_Enter),
        std::make_shared<ImIOWrappers::ImButtonInt>("Cancel",ImIOWrappers::buttonCancel,ImGuiKey_Escape) });
}

#ifdef MOLFLOW // TODO switch to polimorphism
void ConvertToOutgassingMapMenuPress() {
    mApp->imWnd->outgassingMap.Show();
    /*
    if (!mApp->outgassingMapWindow) mApp->outgassingMapWindow = new OutgassingMapWindow();
    mApp->outgassingMapWindow->Display(&mApp->worker);
    */
}
#endif

static void ShowMenuFacet() {
    if (ImGui::MenuItem("Delete", mApp->imWnd->ctrlText+"+DEL")) {
        FacetDeleteMenuPress();
    }
    if (ImGui::MenuItem("Swap normal", mApp->imWnd->ctrlText+"+N")) {
        SwapNormalMenuPress();
    }
    if (ImGui::MenuItem("Shift indices", mApp->imWnd->ctrlText+"+H")) {
        ShiftIndicesMenuPress();
    }
    if (ImGui::MenuItem("Facet coordinates ...")) {
        FacetCoordinatesMenuPress();
    }
    if (ImGui::MenuItem("Move ...")) {
        mApp->imWnd->facetMov.Show();
    }
    if (ImGui::MenuItem("Scale ...")) {
        FacetScaleMenuPress();
    }
    if (ImGui::MenuItem("Mirror / Project ...###MPF")) {
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
        ExplodeMenuPress();
    }
    if (ImGui::MenuItem("Revert flipped normals (old geometries)")) {
        RevertMenuPress();
    }
    if (ImGui::MenuItem("Triangulate")) {
        TriangulateMenuPress();
    }
#ifdef MOLFLOW //convert to polimophism
    if (ImGui::MenuItem("Convert to outgassing map...")) {
        ConvertToOutgassingMapMenuPress();
    }
#endif
}

void ConvexHullMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        try {
            interfGeom->CreatePolyFromVertices_Convex();
        }
        catch (const std::exception& e) {
            mApp->imWnd->popup.Open("Error creating polygon", e.what(), {
                std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter)
                });
        }
        mApp->worker.MarkToReload();
    }
}
void SelectionOrderMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        try {
            interfGeom->CreatePolyFromVertices_Order();
        }
        catch (const std::exception& e) {
            mApp->imWnd->popup.Open("Error creating polygon", e.what(), {
                std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter)
                });
        }
        mApp->worker.MarkToReload();
    }
}

void ClearIsolatedMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    if(interfGeom->GetNbIsolatedVertices()) { //First pass, prevents unnecessary simulation reset
        if (mApp->AskToReset()) { //can renumber facet indices
            interfGeom->DeleteIsolatedVertices(false);
            mApp->UpdateModelParams();
            if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
            if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
            if (mApp->imWnd && mApp->imWnd->vertCoord.IsVisible()) mApp->imWnd->vertCoord.UpdateFromSelection();
            if (mApp->imWnd && mApp->imWnd->facCoord.IsVisible()) mApp->imWnd->facCoord.UpdateFromSelection();
            interfGeom->BuildGLList();
        }
    }
}

void RemoveSelectedMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        auto Y = []() {
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

                if (mApp->imWnd && mApp->imWnd->vertCoord.IsVisible()) mApp->imWnd->vertCoord.UpdateFromSelection();
                if (mApp->imWnd && mApp->imWnd->facCoord.IsVisible()) mApp->imWnd->facCoord.UpdateFromSelection();

                // Send to sub process
                mApp->worker.MarkToReload();
            }
        };
        mApp->imWnd->popup.Open("Remove verices", "Remove Selected vertices?\nThis will also removed the facets containing them!", { 
            std::make_shared<ImIOWrappers::ImButtonFunc>("OK", Y, ImGuiKey_Enter), 
            std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape)
            });
    }
    else {
        mApp->imWnd->popup.Open("Error", "No geometry loaded", { 
            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
            });
    }
}
void VertexCoordinatesMenuPress() {
    mApp->imWnd->vertCoord.Show();
    /*
    if (!mApp->vertexCoordinates) mApp->vertexCoordinates = new VertexCoordinates();
    mApp->vertexCoordinates->Display(&mApp->worker);
    mApp->vertexCoordinates->SetVisible(true);
    */
}
void VertexMoveMenuPress() {
    mApp->imWnd->vertMov.Show();
    /*
    if (interfGeom->IsLoaded()) {
        if (!mApp->moveVertex) mApp->moveVertex = new MoveVertex(interfGeom, &mApp->worker);
        mApp->moveVertex->SetVisible(true);
    }
    else {
        mApp->imWnd->popup.Open("Error", "No geometry loaded", { 
            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
            });
    }
    */
}
void VertexScaleMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->scaleVertex) mApp->scaleVertex = new ScaleVertex(interfGeom, &mApp->worker);
        mApp->scaleVertex->SetVisible(true);
    }
    else {
        mApp->imWnd->popup.Open("Error", "No geometry loaded", { 
            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
            });
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
void VertexAddNewMenuPress() {
    
    if (interfGeom->IsLoaded()) {
        if (!mApp->addVertex) mApp->addVertex = new AddVertex(interfGeom, &mApp->worker);
        mApp->addVertex->SetVisible(true);
    }
    else {
        mApp->imWnd->popup.Open("Error", "No geometry loaded", { 
            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
            });
    }
}

void VertexCoplanarMenuPress() {
    if (interfGeom->IsLoaded()) {
        if (interfGeom->GetNbSelectedVertex() != 3) {
            mApp->imWnd->popup.Open("Error", "Can't define plane, Select exactly 3 vertices", { 
                std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
                });
        }
        else {
            auto F = [](std::string arg) {
                if (!Util::getNumber(&mApp->coplanarityTolerance, arg)) {
                    mApp->imWnd->popup.Open("Error", "Invalid Number", { 
                        std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
                        });
                } else {
                    try { mApp->viewers[mApp->curViewer]->SelectCoplanar(mApp->coplanarityTolerance); }
                    catch (const std::exception& ) {
                        mApp->imWnd->popup.Open("Error", "Error selecting coplanar vertices", { 
                            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
                            });
                    }
                }
            };
            mApp->imWnd->input.Open("Select Coplanar Vertices", "Tolerance(cm)", F, fmt::format("{:.3g}", mApp->coplanarityTolerance));
        }
    }
    else {
        mApp->imWnd->popup.Open("Error", "No geometry loaded", { 
            std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter) 
            });
    }
}

static void ShowMenuVertex() {
    if (ImGui::BeginMenu("Create Facet from Selected")) {
        if (ImGui::MenuItem("Convex Hull", "Alt+V")) {
            ConvexHullMenuPress();
        }
        if (ImGui::MenuItem("Keep selection order")) {
            SelectionOrderMenuPress();
        }
        ImGui::EndMenu();
    }

    if (ImGui::MenuItem("Clear isolated")) {
        ClearIsolatedMenuPress();
    }
#ifdef MOLFLOW
    if (ImGui::MenuItem("Remove selected")) {
        RemoveSelectedMenuPress();
    }
#endif
    if (ImGui::MenuItem("Vertex coordinates...")) {
        VertexCoordinatesMenuPress();
    }
    if (ImGui::MenuItem("Move...")) {
        VertexMoveMenuPress();
    }
    if (ImGui::MenuItem("Scale...")) {
        VertexScaleMenuPress();
    }
    if (ImGui::MenuItem("Mirror / Project ...")) {
        VertexMirrorProjectMenuPress();
    }
    if (ImGui::MenuItem("Rotate...")) {
        VertexRotateMenuPress();
    }
    if (ImGui::MenuItem("Add new...")) {
        VertexAddNewMenuPress();
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select all vertex")) {
        interfGeom->SelectAllVertex();
    }
    if (ImGui::MenuItem("Unselect all vertex")) {
        interfGeom->UnselectAllVertex();
    }
    if (ImGui::MenuItem("Select coplanar vertex (visible on screen)")) {
        VertexCoplanarMenuPress();
    }
    if (ImGui::MenuItem("Select isolated vertex")) {
        interfGeom->SelectIsolatedVertices();
    }
}

void ShowPreviousStructureMenuPress() {
    if (interfGeom->viewStruct == -1) interfGeom->viewStruct = static_cast<int>(interfGeom->GetNbStructure() - 1);
    else
        interfGeom->viewStruct = (int)Previous(interfGeom->viewStruct, interfGeom->GetNbStructure());
    interfGeom->UnselectAll();
}

void ShowNextStructureMenuPress() {
    interfGeom->viewStruct = (int)Next(interfGeom->viewStruct, interfGeom->GetNbStructure());
    interfGeom->UnselectAll();
}

void UpdateViewShortcuts() {
    mApp->imWnd->shortcutMan.UnregisterShortcut(7);
    std::vector<int> keys = {
        SDL_SCANCODE_F1,
        SDL_SCANCODE_F2,
        SDL_SCANCODE_F3,
        SDL_SCANCODE_F5,
        SDL_SCANCODE_F6,
        SDL_SCANCODE_F7,
        SDL_SCANCODE_F8,
        SDL_SCANCODE_F9,
        SDL_SCANCODE_F10,
    };
    for (int i = 0; i < mApp->views.size() && i < keys.size(); i++) {
        std::function<void()> F = [i]() { mApp->SelectView(i); };
        mApp->imWnd->shortcutMan.RegisterShortcut({SDL_SCANCODE_LALT,keys.at(i)}, F, 6);
    }
}

void AddNewViewMenuPress() {
    LockWrapper myLock(mApp->imguiRenderLock);
    mApp->AddView();
    UpdateViewShortcuts();
}

void UpdateStructuresShortcuts() {
    mApp->imWnd->shortcutMan.UnregisterShortcut(6);
    if (!interfGeom) return;
    for (int i = 0; i < interfGeom->GetNbStructure() && i+2<12; i++) {
        std::function<void()> F = [i]() { interfGeom->viewStruct = i; };
        mApp->imWnd->shortcutMan.RegisterShortcut({mApp->imWnd->modifier, SDL_SCANCODE_F2+i},F,6);
    }
}


static void ShowMenuView() {
    if (ImGui::BeginMenu("Structure")) {
        if (ImGui::MenuItem("New structure...")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->AddStruct();
            UpdateStructuresShortcuts();
        }
        if (ImGui::MenuItem("Delete structure...")) {
            LockWrapper myLock(mApp->imguiRenderLock);
            mApp->DeleteStruct();
            UpdateStructuresShortcuts();
        }
        ImGui::Separator();
        if (ImGui::MenuItem("Show All", mApp->imWnd->ctrlText+"+F1")) {
            interfGeom->viewStruct = -1; // -1 will show all, number of structures is interfGeom->GetNbStructure()
        }
        if (ImGui::MenuItem("Show Previous", mApp->imWnd->ctrlText+"+F11")) {
            ShowPreviousStructureMenuPress();
        }
        if (ImGui::MenuItem("Show Next", mApp->imWnd->ctrlText+"+F12")) { 
            ShowNextStructureMenuPress();
        }
        ImGui::Separator();
        // Procedural list of memorized structures
        for (int i = 0; i < interfGeom->GetNbStructure(); i++) {
            if ( ImGui::MenuItem("Show #" + std::to_string(i+1) + " (" + interfGeom->GetStructureName(i) + ")", i+2 <=10 ? mApp->imWnd->ctrlText+" + F" + std::to_string(i + 2) : "")) {
                interfGeom->viewStruct = i;
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem(ICON_FA_TH_LARGE "  Full Screen###Full Screen")) {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->ToggleFullscreen();
    }
    ImGui::Separator();

    if (ImGui::BeginMenu("Memorize view to")) {
        if (ImGui::MenuItem("Add new...", mApp->imWnd->ctrlText+"+Q")) {
            AddNewViewMenuPress();
        }
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name)) {
                LockWrapper lw(mApp->imguiRenderLock);
                mApp->OverWriteView(i);
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    if (ImGui::BeginMenu("Select memorized", mApp->views.size()!=0)) {
        // Procedural list of memorized views
        std::vector<std::string> shortcuts = { "F1", "F2", "F3", "F5", "F6", "F7", "F8", "F9", "F10" };
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name, i < shortcuts.size() ? "Alt+" + shortcuts[i] : "")) {
                mApp->SelectView(i);
            }
        }
        ImGui::EndMenu();
    }
    if (ImGui::BeginMenu("Clear memorized")) {
        if (ImGui::MenuItem("Clear All")) {
            auto Y = []() {LockWrapper myLock(mApp->imguiRenderLock); mApp->ClearAllViews(); UpdateViewShortcuts(); };
            mApp->imWnd->popup.Open("Clear all views?", "Are you sure you want to clear(forget) all remembered views?", {
                std::make_shared<ImIOWrappers::ImButtonFunc>("Ok", Y, ImGuiKey_Enter), 
                std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape)
                });
        }
        // Procedural list of memorized views
        for (int i = 0; i < mApp->views.size(); i++) {
            if (ImGui::MenuItem(mApp->views[i].name)) {
                mApp->ClearView(i);
                UpdateViewShortcuts();
            }
        }
        ImGui::EndMenu();
    }
}

static void QuickPipeMenuPress() {
    auto common = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(5, 5); };
    ImIOWrappers::AskToSaveBeforeDoing(common);
}

static void ShowMenuTest() {
    if (ImGui::MenuItem("Pipe (L/R=0.0001)")) {
        auto common = []() { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(0.0001, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    if (ImGui::MenuItem("Pipe (L/R=1)")) {
        auto common = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1.0, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    if (ImGui::MenuItem("Pipe (L/R=10)")) {
        auto common = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10.0, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    if (ImGui::MenuItem("Pipe (L/R=100)")) {
        auto common = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(100.0, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    if (ImGui::MenuItem("Pipe (L/R=1000)")) {
        auto common = []() -> void { LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(1000.0, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    if (ImGui::MenuItem("Pipe (L/R=10000)")) {
        auto common = []() -> void {  LockWrapper myLock(mApp->imguiRenderLock); mApp->BuildPipe(10000.0, 0); };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    //Quick test pipe
    ImGui::Separator();
    if (ImGui::MenuItem("Quick Pipe", "Alt+Q")) {
        QuickPipeMenuPress();
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Triangulate Geometry")) {
        auto common = []() -> void {
            LockWrapper myLock(mApp->imguiRenderLock);
            auto prg = GLProgress_GUI("Triangulating", "Triangulating");
            prg.SetVisible(true);
            GeometryTools::PolygonsToTriangles(mApp->worker.GetGeometry(), prg);
            mApp->worker.MarkToReload();
        };
        ImIOWrappers::AskToSaveBeforeDoing(common);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("ImGui Menu")) {
        mApp->imWnd->ToggleMainMenu();
    }
    if (ImGui::MenuItem("ImGui Test Suite")) {
        mApp->imWnd->ToggleMainHub();
    }
#ifdef ENABLE_IMGUI_TESTS
    if (ImGui::MenuItem("ImGui Test Engine")) {
        mApp->imWnd->testEngine.Show();
    }
#endif

    if (ImGui::MenuItem("Shortcut Test", mApp->imWnd->ctrlText+"+T")) {
        ImIOWrappers::InfoPopup("Menu Shortcut", "Menu Shortcut");
    }
}

#ifdef MOLFLOW // TODO polyporphysm
static void ShowMenuTime() {
    if (ImGui::MenuItem("Time settings...", "Alt+I")) { //TODO ImGui time settings + shortcut
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
    }
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
// static (not changing at runtime) shortcuts
void RegisterShortcuts() {
#if defined(__MACOSX__) || defined(__APPLE__)
    mApp->imWnd->ctrlText = "CMD";
    mApp->imWnd->modifier = ImGuiKey_LeftSuper;
#endif
    if (!interfGeom) interfGeom = mApp->worker.GetGeometry();
    std::function<void()> ControlO = []() { ImMenu::LoadMenuButtonPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_O }, ControlO);

    std::function<void()> ControlS = []() {if (mApp->worker.GetGeometry()->IsLoaded()) ImIOWrappers::DoSave(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_S }, ControlS);
    
    std::function<void()> Altf4 = []() { ImMenu::QuitMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_F4 }, Altf4);

    std::function<void()> AltS = []() { mApp->imWnd->smartSelect.Show(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_S }, AltS);

    std::function<void()> ControlA = []() { interfGeom->SelectAll(); mApp->UpdateFacetParams(true); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_A }, ControlA);

    std::function<void()> AltN = []() { mApp->imWnd->selByNum.Show(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_N }, AltN);

    std::function<void()> ControlI = []() { ImMenu::InvertSelectionMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_I }, ControlI);
    
    std::function<void()> ControlW = []() { ImMenu::NewSelectionMemoryMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_W }, ControlW);

    std::function<void()> Altf11 = []() { mApp->SelectSelection(Previous(mApp->idSelection, mApp->selections.size())); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_F11 }, Altf11);

    std::function<void()> Altf12 = []() { mApp->SelectSelection(Next(mApp->idSelection, mApp->selections.size())); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_F12 }, Altf12);

    std::function<void()> AltF = []() { ImMenu::FormulaEditorMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_F }, AltF);

    std::function<void()> AltC = []() { ImMenu::ConvergencePlotterMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_C }, AltC);

    std::function<void()> AltT = []() { ImMenu::TexturePlotterMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_T }, AltT);

    std::function<void()> AltP = []() { ImMenu::ProfilePlotterMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_P }, AltP);

    std::function<void()> ControlD = []() { ImMenu::TextureScalingMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_D }, ControlD);

    std::function<void()> ControlR = []() { ImMenu::TakeScreenshotMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_R }, ControlR);

    std::function<void()> ControlDel = []() { ImMenu::FacetDeleteMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_DELETE }, ControlDel);

    std::function<void()> ControlN = []() { ImMenu::SwapNormalMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_N }, ControlN);

    std::function<void()> ControlH = []() { ImMenu::ShiftIndicesMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_H }, ControlH);

    std::function<void()> ControlF1 = []() { interfGeom->viewStruct = -1; };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_F1 }, ControlF1);

    std::function<void()> ControlF11 = []() { ImMenu::ShowPreviousStructureMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_F11 }, ControlF11);

    std::function<void()> ControlF12 = []() { ImMenu::ShowNextStructureMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_F12 }, ControlF12);

    std::function<void()> ControlQ = []() { ImMenu::AddNewViewMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ mApp->imWnd->modifier, SDL_SCANCODE_Q }, ControlQ);

    std::function<void()> AltQ = []() { ImMenu::QuickPipeMenuPress(); };
    mApp->imWnd->shortcutMan.RegisterShortcut({ SDL_SCANCODE_LALT, SDL_SCANCODE_Q }, AltQ);
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

    static bool firstDraw = true;
    if (firstDraw) {
        ImMenu::UpdateSelectionShortcuts();
        ImMenu::UpdateViewShortcuts();
        ImMenu::UpdateStructuresShortcuts();
#if defined(__MACOSX__) || defined(__APPLE__)
        mApp->imWnd->ctrlText = "CMD";
        mApp->imWnd->modifier = ImGuiKey_LeftSuper;
#endif
        static bool firstDraw = false;
    }

    if (!interfGeom)
        interfGeom = mApp->worker.GetGeometry();

    static float verticalMainMenuBarSize = 5.f;
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, verticalMainMenuBarSize));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y + verticalMainMenuBarSize));
    if (ImGui::BeginMainMenuBar()) {
        ImGui::PopStyleVar(2);
        ImGui::AlignTextToFramePadding();
        if (ImGui::BeginMenu(ICON_FA_FILE_ARCHIVE "  File###File")) {
            ImMenu::ShowMenuFile();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_MOUSE_POINTER "  Selection###Selection")) {
            ImMenu::ShowMenuSelection();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu(ICON_FA_TOOLS "  Tools###Tools")) {
            ImMenu::ShowMenuTools();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Facet")) {
            ImMenu::ShowMenuFacet();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Vertex")) {
            ImMenu::ShowMenuVertex();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("View")) {
            ImMenu::ShowMenuView();
            ImGui::EndMenu();
        }
        if (ImGui::BeginMenu("Test")) {
            ImMenu::ShowMenuTest();
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
}
}

void ImExplodeFacet::Draw()
{
    if (!drawn) return;
    ImGuiIO& io = ImGui::GetIO();
    ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
    ImGui::Begin("Explode?", &drawn, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings);
    ImGui::TextWrapped("Are you sure you want to explode selected facets?");
    if (ImGui::Button("  Yes  ") || io.KeysDown[SDL_SCANCODE_RETURN] || io.KeysDown[SDL_SCANCODE_KP_ENTER]) {
        DoExplode();
        this->Hide();
    } ImGui::SameLine();
    if (ImGui::Button("  Cancel  ") || io.KeysDown[SDL_SCANCODE_ESCAPE]) {
        this->Hide();
    }
    ImGui::End();
}

void ImExplodeFacet::DoExplode()
{
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        int err;
        try {
            err = interfGeom->ExplodeSelected();
        }
        catch (const std::exception&) {
            ImIOWrappers::InfoPopup("Error", "Error Exploding");
        }
        if (err == -1) {
            ImIOWrappers::InfoPopup("Error", "Empty Selection");
        }
        else if (err == -2) {
            ImIOWrappers::InfoPopup("Error", "All selected facets must have a texture");
        }
        else if (err == 0) {
            mApp->UpdateModelParams();
            mApp->UpdateFacetParams(true);
            // Send to sub process
            mApp->worker.MarkToReload();
        }
    }
}
