//
// Created by pbahr on 19/03/2021.
//

#include "ImguiMenu.h"
#include "imgui/imgui.h"
#include <imgui/IconsFontAwesome5.h>
#include <Helper/MathTools.h>

#include "SmartSelection.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "SelectFacetByResult.h"


#include "Worker.h"
#include "Geometry_shared.h"
#include "Facet_shared.h"

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

static void ShowMenuFile() {
    ImGui::MenuItem("New, empty geometry");
    ImGui::MenuItem("Load", "Ctrl+O");
    ImGui::MenuItem("Load recent");
    ImGui::Separator();
    if (ImGui::BeginMenu("Insert geometry")) {
        ImGui::MenuItem("To current structure");
        ImGui::MenuItem("To new structure");
        ImGui::EndMenu();
    }
    ImGui::Separator();
    ImGui::MenuItem("Save", "Ctrl+S");
    ImGui::MenuItem("Save as");
    ImGui::Separator();

    ImGui::MenuItem("Export selected facets");

    ImGui::MenuItem("Export selected profiles");

    //ImGui::MenuItem("File")->SetIcon(MENU_FILE_SAVE, 83, 24);
    //ImGui::MenuItem("File")->SetIcon(MENU_FILE_SAVEAS, 101, 24);
    //ImGui::MenuItem("File")->SetIcon(MENU_FILE_LOAD, 65, 24);//65,24
    //ImGui::MenuItem("File")->SetIcon(MENU_FILE_LOADRECENT,83,24);//83,24

    // TODO: Molflow only entries

    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {
    }
}

static void ShowSelectionModals() {

    Worker &worker = mApp->worker;
    Geometry *geom = worker.GetGeometry();

    if (ImGui::BeginPopupModal("Select large facets without hits", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

        char *input;
        char tmp[128];
        double largeAreaThreshold = 1.0;
        sprintf(tmp, "%g", largeAreaThreshold);

        ImGui::InputDouble("Min.area (cm\262)", &largeAreaThreshold, 0.0f, 0.0f,
                           "%lf");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            geom->UnselectAll();
            for (int i = 0; i < geom->GetNbFacet(); i++)
                if (geom->GetFacet(i)->facetHitCache.hit.nbMCHit == 0 &&
                    geom->GetFacet(i)->sh.area >= largeAreaThreshold)
                    geom->SelectFacet(i);
            geom->UpdateSelection();
            mApp->UpdateFacetParams(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Select non planar facets", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

        char *input;
        char tmp[128];
        static double planarityThreshold = 1e-5;
        sprintf(tmp, "%g", planarityThreshold);

        ImGui::InputDouble("Planarity larger than:", &planarityThreshold, 0.0f, 0.0f,
                           "%lf");
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            geom->UnselectAll();
            std::vector<size_t> nonPlanarFacetids = geom->GetNonPlanarFacetIds(planarityThreshold);
            for (const auto &i : nonPlanarFacetids)
                geom->SelectFacet(i);
            geom->UpdateSelection();
            mApp->UpdateFacetParams(true);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (ImGui::BeginPopupModal("Enter selection name", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

        char *input;
        char selectionName[128];

        std::stringstream tmp;
        tmp << "Selection #" << (mApp->selections.size() + 1);
        strcpy(selectionName, tmp.str().c_str());
        ImGui::InputText("Selection name", selectionName, 128);
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            if (strcmp(selectionName, "") != 0)
                mApp->AddSelection(selectionName);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // TODO: with sel id

    if (ImGui::BeginPopupModal("Clear all selections ?", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            mApp->ClearAllSelections();
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    // TODO: for clear id
    /*char tmpname[256];
    sprintf(tmpname, "Clear %s?", mApp->selections[i].name.c_str());
    if (ImGui::BeginPopupModal(tmpname, NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        if (ImGui::Button("OK", ImVec2(120, 0))) {
            mApp->ClearSelection(i);
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::SameLine();
        if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }*/
}

static void ShowMenuSelection(std::string& openModalName) {
    static SmartSelection *smartSelection = nullptr;
    static SelectDialog *selectDialog = nullptr;
    static SelectTextureType *selectTextureType = nullptr;
    static SelectFacetByResult *selectFacetByResult = nullptr;

    //std::string openmodalName = "";


    Worker &worker = mApp->worker;
    Geometry *geom = worker.GetGeometry();

    if (ImGui::MenuItem("Smart Select facets...", "ALT+S")) {
        if (!smartSelection) smartSelection = new SmartSelection(worker.GetGeometry(), &worker);
        smartSelection->SetVisible(true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select All Facets", "CTRL+A")) {
        geom->SelectAll();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select by Facet Number...", "ALT+N")) {
        if (!selectDialog) selectDialog = new SelectDialog(worker.GetGeometry());
        selectDialog->SetVisible(true);
    }
    if (ImGui::MenuItem("Select Sticking", "")) {
        // TODO: Different for Synrad?
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.sticking_paramId != -1 ||
                (geom->GetFacet(i)->sh.sticking != 0.0 && !geom->GetFacet(i)->IsTXTLinkFacet()))
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Transparent", "")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (
#if defined(MOLFLOW)
geom->GetFacet(i)->sh.opacity_paramId != -1 ||
#endif
(geom->GetFacet(i)->sh.opacity != 1.0 && geom->GetFacet(i)->sh.opacity != 2.0))
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    if (ImGui::MenuItem("Select 2 sided")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.is2sided)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Texture")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.isTextured)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select by Texture type...")) {
        if (!selectTextureType) selectTextureType = new SelectTextureType(&worker);
        selectTextureType->SetVisible(true);
    }
    if (ImGui::MenuItem("Select Profile")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.isProfile)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    ImGui::Separator();
    if (ImGui::MenuItem("Select Abs > 0")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++) {
            if (geom->GetFacet(i)->facetHitCache.hit.nbAbsEquiv > 0)
                geom->SelectFacet(i);
        }
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Hit > 0")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->facetHitCache.hit.nbMCHit > 0)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select large with no hits...")) {
        openModalName="Select large facets without hits";
    }
    if (ImGui::MenuItem("Select by facet result...")) {
        if (!selectFacetByResult) selectFacetByResult = new SelectFacetByResult(&worker);
        selectFacetByResult->SetVisible(true);
    }
    ImGui::Separator();
    if (ImGui::MenuItem("Select link facets")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)

            if (geom->GetFacet(i)->sh.superDest != 0)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select teleport facets")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)

            if (geom->GetFacet(i)->sh.teleportDest != 0)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select non planar facets")) {
        openModalName="Select non planar facets";
    }
    if (ImGui::MenuItem("Select non simple facets")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)

            if (geom->GetFacet(i)->nonSimple)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    //if(ImGui::MenuItem(nullptr) {} // Separator
    //if(ImGui::MenuItem("Load selection",MENU_FACET_LOADSEL) {}
    //if(ImGui::MenuItem("Save selection",MENU_FACET_SAVESEL) {}
    if (ImGui::MenuItem("Invert selection", "CTRL+I")) {
        for (int i = 0; i < geom->GetNbFacet(); i++)
            geom->GetFacet(i)->selected = !geom->GetFacet(i)->selected;
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    ImGui::Separator();
    if (ImGui::BeginMenu("Memorize selection to")) {
        if (ImGui::MenuItem("Add new...", "CTRL+W")) {
            openModalName="Enter selection name";
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                /*if (ImGui::BeginPopupModal("Enter selection name", NULL,
                                           ImGuiWindowFlags_AlwaysAutoResize)) {

                    char *input;
                    char selectionName[128];

                    ImGui::InputText("Selection name", selectionName, 128);
                    ImGui::Separator();

                    if (strcmp(selectionName, "") != 0) return;
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        mApp->selections[i].selection = geom->GetSelectedFacets();
                        mApp->selections[i].name = selectionName;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }*/
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
        }
        ImGui::Separator();
        for (size_t i = 0; i < mApp->selections.size(); i++) {
            if (ImGui::MenuItem(mApp->selections[i].name.c_str())) {
                /*char tmpname[256];
                sprintf(tmpname, "Clear %s?", mApp->selections[i].name.c_str());
                if (ImGui::BeginPopupModal(tmpname, NULL,
                                           ImGuiWindowFlags_AlwaysAutoResize)) {
                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        mApp->ClearSelection(i);
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }*/
            }
        }
        ImGui::EndMenu();
    }
    ImGui::Separator();

    // TODO: Extract Molflow only entries
    if (ImGui::MenuItem("Select Desorption")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.desorbType != DES_NONE)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Outgassing Map")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->hasOutgassingFile)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select Reflective")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++) {
            InterfaceFacet *f = geom->GetFacet(i);
            if (f->sh.desorbType == DES_NONE && f->sh.sticking == 0.0 && f->sh.opacity > 0.0)
                geom->SelectFacet(i);
        }
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }
    if (ImGui::MenuItem("Select volatile facets")) {
        geom->UnselectAll();
        for (int i = 0; i < geom->GetNbFacet(); i++)
            if (geom->GetFacet(i)->sh.isVolatile)
                geom->SelectFacet(i);
        geom->UpdateSelection();
        mApp->UpdateFacetParams(true);
    }

    if (ImGui::MenuItem("Shortcut test", "CTRL+Q")) {
        openModalName="testmod";
    }
}


static void ShowMenuTools() {
    if (ImGui::MenuItem("Formula editor", "ALT+F")) {}
    if (ImGui::MenuItem("Convergence Plotter ...", "ALT+C")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Texture Plotter ...", "ALT+T")) {}
    if (ImGui::MenuItem("Profile Plotter ...", "ALT+P")) {}
#if defined(MOLFLOW)
    if (ImGui::MenuItem("Histogram Plotter...")) {}
#endif
    ImGui::Separator();
    if (ImGui::MenuItem("Texture scaling...", "CTRL+D")) {}
    if (ImGui::MenuItem("Particle logger...")) {}
    //if (ImGui::MenuItem("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER)){}
    if (ImGui::MenuItem("Global Settings ...")) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Take screenshot", "CTRL+R")) {}


    // TODO: Extract Molflow only entries
    ImGui::Separator();
    if (ImGui::MenuItem("Moving parts...")) {}
}

static void ShowMenuFacet() {
    if (ImGui::MenuItem("Delete", "CTRL+DEL")) {}
    if (ImGui::MenuItem("Swap normal", "CTRL+N")) {}
    if (ImGui::MenuItem("Shift indices", "CTRL+H")) {}
    if (ImGui::MenuItem("Facet coordinates ...")) {}
    if (ImGui::MenuItem("Move ...")) {}
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
    /*if (ImGui::GetIO().KeyCtrl && ImGui::IsKeyPressed(SDL_SCANCODE_Q)){
        ImGui::OpenPopup("testmod");
    }*/

    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 16.f));
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y + 16.0f));
    static std::string openmodalName = "";
    if (ImGui::BeginMainMenuBar()) {
        ImGui::AlignTextToFramePadding();
        if (ImGui::BeginMenu(ICON_FA_FILE_ARCHIVE "  File")) {
            ShowMenuFile();
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
    ShowSelectionModals();

    //ImGui::PopStyleVar(1);
}