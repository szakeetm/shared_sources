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
#include <Interface.h>
//#include <direct.h> //_getcwd()
//#include <io.h> // Check for recovery

#ifdef _WIN32

#else
//#include <sys/sysinfo.h>
#include <unistd.h>
#endif

#include <filesystem>
#include <string>
#include <AppUpdater.h>

//#include "GLApp/GLFileBox.h"
#include "NativeFileDialog/molflow_wrapper/nfd_wrapper.h"
#include "GLApp/GLMessageBox.h"
#include "GLApp/GLInputBox.h"
#include "GLApp/GLSaveDialog.h"
#include "GLApp/GLToolkit.h"
#include "Helper/MathTools.h" //IDX
#include "RecoveryDialog.h"
#include "Facet_shared.h"

#include "GLApp/GLTextField.h"
#include "GLApp/GLToggle.h"
#include "GLApp/GLLabel.h"
#include "GLApp/GLButton.h"
#include "GLApp/GLTitledPanel.h"
#include "GLApp/GLList.h"
#include "GLApp/GLCombo.h"
#include "GLApp/GLMenuBar.h"
#include "GLApp/GLWindowManager.h"

//Windows
#include "CollapseSettings.h"
#include "HistogramSettings.h"
#include "HistogramPlotter.h"
#include "ConvergencePlotter.h"
#include "MoveVertex.h"
#include "ScaleVertex.h"
#include "ScaleFacet.h"
#include "MoveFacet.h"
#include "CreateShape.h"
#include "ExtrudeFacet.h"
#include "MirrorFacet.h"
#include "MirrorVertex.h"
#include "SplitFacet.h"
#include "BuildIntersection.h"
#include "RotateFacet.h"
#include "RotateVertex.h"
#include "FacetCoordinates.h"
#include "VertexCoordinates.h"
#include "SmartSelection.h"
#include "SelectDialog.h"
#include "SelectTextureType.h"
#include "SelectFacetByResult.h"
#include "AlignFacet.h"
#include "AddVertex.h"
#include "FormulaEditor.h"
#include "ParticleLogger.h"
#include "CrossSection.h"

//#include "NativeFileDialog/nfd.h"

//Updater
#include "File.h" //File utils (Get extension, etc)

//Test functions
#include "GeometryTools.h"
#include "Helper/StringHelper.h" //abbreviate long file paths in recent menus
#include "Helper/FormatHelper.h" //unit formatting

#include "../../src/versionId.h"
#include "ImguiWindow.h"

extern Worker worker;
extern std::vector<std::string> formulaPrefixes;
//extern const char* appTitle;

/*
extern const char *fileLFilters;
extern const char *fileInsFilters;
extern const char *fileSFilters;
extern const char *fileDesFilters;
*/
extern char fileLFilters[];
extern char fileInsFilters[];
extern char fileSaveFilters[];
extern char fileSelFilters[];
extern char fileTexFilters[];
extern char fileDesFilters[];


extern int cSize;
extern int cWidth[];
extern const char *cName[];


Interface::Interface() : GLApplication(){
    //Get number of cores
#ifdef _WIN32
    compressProcessHandle = nullptr;
    SYSTEM_INFO sysinfo;
    GetSystemInfo(&sysinfo);
    numCPU = (size_t) sysinfo.dwNumberOfProcessors;
#else
    numCPU = (unsigned int)sysconf(_SC_NPROCESSORS_ONLN);
#endif
    appUpdater = nullptr; //We'll initialize later, when the app name and version id is known

    antiAliasing = true;
    whiteBg = false;
    highlightNonplanarFacets = true;
    highlightSelection = true;
    leftHandedView = false;
    autoUpdateFormulas = true;
    compressSavedFiles = true;
    /*double sp.gasMass=28;
    double totalOutgassing=0.0; //total outgassing in Pa*m3/sec (internally everything is in SI units)
    double totalInFlux = 0.0; //total incoming molecules per second. For anisothermal system, it is (totalOutgassing / Kb / T)*/
    autoSaveFrequency = 10.0; //in minutes
    autoSaveSimuOnly = false;
    autosaveFilename = "";
    autoFrameMove = true;

    lastSaveTime = 0.0f;
    lastSaveTimeSimu = 0.0f;
    changedSinceSave = false;
    //lastHeartBeat=0.0f;
    nbDesStart = 0;
    nbHitStart = 0;

    lastUpdate = 0.0;
    //nbFormula = 0;

    idSelection = 0;

#if defined(_DEBUG) || defined(DEBUG)
    nbProc = 1;
#else
    nbProc = numCPU; //numCPU also displayed in Global Settings
    Saturate(nbProc, 1, (size_t)16); //don't start with more than 16 processes, but the user can increase
#endif

    curViewer = 0;

    imWnd = nullptr;

    m_strWindowTitle = appTitle;
    wnd->SetBackgroundColor(212, 208, 200);
    m_bResizable = true;
    m_minScreenWidth = 800;
    m_minScreenHeight = 600;
    coplanarityTolerance = 1e-8;
    largeAreaThreshold = 1.0;
    planarityThreshold = 1e-5;

    updateRequested = true;
    //prevRunningState = false;
}

Interface::~Interface() {
    SAFE_DELETE(menu);
    SAFE_DELETE(appUpdater);
}

void Interface::UpdateViewerFlags() {
    viewers[curViewer]->showNormal = showNormal->GetState();
    viewers[curViewer]->showRule = showRule->GetState();
    viewers[curViewer]->showUV = showUV->GetState();
    viewers[curViewer]->showLeak = showLeak->GetState();
    viewers[curViewer]->showHit = showHit->GetState();
    viewers[curViewer]->showLine = showLine->GetState();
    viewers[curViewer]->showVolume = showVolume->GetState();
    viewers[curViewer]->showTexture = showTexture->GetState();
    viewers[curViewer]->showFacetId = showFacetId->GetState();

    bool neededTexture = needsTexture;
    CheckNeedsTexture();

    if (!needsTexture && neededTexture) { //We just disabled mesh
        worker.GetGeometry()->ClearFacetTextures();
    } else if (needsTexture && !neededTexture) { //We just enabled mesh
        worker.RebuildTextures();
    }
    viewers[curViewer]->showFilter = false;//showFilter->GetState();
    viewers[curViewer]->showVertexId = showVertexId->GetState();
    viewers[curViewer]->showIndex = showIndex->GetState();
}

void Interface::ResetSimulation(bool askConfirm) {

    bool ok = true;
    if (askConfirm)
        ok = GLMessageBox::Display("Reset simulation ?", "Question", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) ==
             GLDLG_OK;

    if (ok) {
        if(Interface::worker.IsRunning()) {
            try {
                this->worker.Stop_Public();
            }
            catch (const std::exception&) {
                ok = GLMessageBox::Display("Could not stop simulation, reset anyway ?", "Question",
                                           GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) == GLDLG_OK;
                if (!ok)
                    return;
            }
        }
        worker.ResetStatsAndHits(m_fTime);
        hps.clear();
        dps.clear();
        hps_runtotal.clear();
        dps_runtotal.clear();

        nbDesStart = 0;
        nbHitStart = 0;
        if (convergencePlotter) {
            convergencePlotter->Refresh();
        }
        if(formulaEditor) formulaEditor->UpdateValues();
        if(particleLogger) particleLogger->UpdateStatus();
    }
    UpdatePlotters();
}

void Interface::UpdateStructMenu() {

    InterfaceGeometry *interfGeom = worker.GetGeometry();

    structMenu->Clear();
    structMenu->Add("New structure...", MENU_VIEW_NEWSTRUCT);
    structMenu->Add("Delete structure...", MENU_VIEW_DELSTRUCT);
    structMenu->Add(nullptr); //Separator
    structMenu->Add("Show all", MENU_VIEW_STRUCTURE, SDLK_F1, CTRL_MODIFIER);
    structMenu->Add("Show previous", MENU_VIEW_PREVSTRUCT, SDLK_F11, CTRL_MODIFIER);
    structMenu->Add("Show next", MENU_VIEW_NEXTSTRUCT, SDLK_F12, CTRL_MODIFIER);
    structMenu->Add(nullptr); //Separator

    for (int i = 0; i < interfGeom->GetNbStructure(); i++) {
        std::string label = fmt::format("Show #{} ({})", i + 1, interfGeom->GetStructureName(i));
        if (i < 10)
            structMenu->Add(label, MENU_VIEW_STRUCTURE + (i + 1), SDLK_F1 + i + 1, CTRL_MODIFIER);
        else
            structMenu->Add(label, MENU_VIEW_STRUCTURE + (i + 1));
    }

    structMenu->SetCheck(MENU_VIEW_STRUCTURE + interfGeom->viewStruct + 1, true);

    UpdateTitle();
}

void Interface::UpdateTitle() {

    std::string title;

    InterfaceGeometry *interfGeom = worker.GetGeometry();

    if (!interfGeom->IsLoaded()) {
        title = appTitle;
    } else {
        if (interfGeom->viewStruct < 0) {
            title = appTitle + " [" + worker.GetCurrentShortFileName() + "]";
        } else {
            title = appTitle + " [" + worker.GetCurrentShortFileName() + ": Struct #" +
                    std::to_string(interfGeom->viewStruct + 1) + " " + interfGeom->GetStructureName(interfGeom->viewStruct) + "]";
        }
    }

    SetTitle(title);
}

void Interface::LoadSelection(const char *fName) {

    std::string fileName = fName;

    if (fileName.empty()) {
        fileName = NFD_OpenFile_Cpp(fileSelFilters, "");
        if (fileName.empty()) return;
    }


    try {

        InterfaceGeometry *interfGeom = worker.GetGeometry();
        interfGeom->UnselectAll();
        size_t nbFacet = interfGeom->GetNbFacet();

        {
            auto file = FileReader(fileName);
            while (!file.IsEof()) {
                int s = file.ReadInt();
                if (s >= 0 && s < nbFacet) interfGeom->SelectFacet(s);
            }
        }
        interfGeom->UpdateSelection();

        UpdateFacetParams(true);
    }
    catch (const std::exception &e) {

        char errMsg[512];
        sprintf(errMsg, "%s\nFile:%s", e.what(), fileName.c_str());
        GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);

    }
    changedSinceSave = false;
}

void Interface::SaveSelection() {

    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (interfGeom->GetNbSelectedFacets() == 0) return;
    auto prg = GLProgress_GUI("Saving file", "Please wait");
    prg.SetVisible(true);
    //GLWindowManager::Repaint();

    std::string fileName = NFD_SaveFile_Cpp(fileSelFilters, "");
    //FILENAME *fn = GLFileBox::SaveFile(currentSelDir, worker.GetCurrentShortFileName(), "Save selection", fileSelFilters, 0);

    if (!fileName.empty()) {

        try {

            if (FileUtils::GetExtension(fileName).empty()) fileName = fileName + ".sel";

            auto file = FileWriter(fileName);
            //int nbSelected = interfGeom->GetNbSelectedFacets();
            size_t nbFacet = interfGeom->GetNbFacet();
            for (size_t i = 0; i < nbFacet; i++) {
                if (interfGeom->GetFacet(i)->selected) file.Write(i, "\n");
            }

        }
        catch (const std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nFile:%s", e.what(), fileName.c_str());
            GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
        }

    }
    changedSinceSave = false;
}

void Interface::ExportSelection() {

    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (interfGeom->GetNbSelectedFacets() == 0) {
        GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }

    std::string fileName = NFD_SaveFile_Cpp(fileSaveFilters, "");
    auto prg = GLProgress_GUI("Saving file...", "Please wait");
    
    prg.SetVisible(true);
    //GLWindowManager::Repaint();
    if (!fileName.empty()) {
        std::string ext = FileUtils::GetExtension(fileName);
        if (ext.empty())
            fileName += (compressSavedFiles ? ".zip"
                                            : ".xml"); //This is also done within worker.SaveGeometry but we need it to add to recents
        try {
            worker.SaveGeometry(fileName, prg, true, true);
            AddRecent(fileName);
        }
        catch (const std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nFile:%s", e.what(), fileName.c_str());
            GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
        }

    }
}

// Name: UpdateModelParams()
// Desc: Update displayed model parameter on geometry change

void Interface::UpdateModelParams() {

    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char tmp[256];
    double sumArea = 0;
    facetList->SetSize(cSize, interfGeom->GetNbFacet(), false, "Clearing facet hit list...");
    facetList->SetColumnWidths((int *) cWidth);
    facetList->SetColumnLabels((const char **) cName);
    UpdateFacetHits(true);
    UpdateFacetlistSelected();
    AxisAlignedBoundingBox bb = interfGeom->GetBB();

    for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
        InterfaceFacet *f = interfGeom->GetFacet(i);
        if (f->sh.area > 0) sumArea += f->GetArea();
    }

    sprintf(tmp, "V:%zd F:%zd Dim:(%g,%g,%g) Area:%g", interfGeom->GetNbVertex(), interfGeom->GetNbFacet(),
            (bb.max.x - bb.min.x), (bb.max.y - bb.min.y), (bb.max.z - bb.min.z), sumArea);
    geomNumber->SetText(tmp);

}

void Interface::AnimateViewerChange(int next) {

    double xs1, ys1, xs2, ys2;
    double xe1, ye1, xe2, ye2;
    int sx = m_screenWidth - 205;
    int fWidth = m_screenWidth - 215;
    int fHeight = m_screenHeight - 27;
    int Width2 = fWidth / 2 - 1;
    int Height2 = fHeight / 2 - 1;

    // Reset to layout and make all visible

    for (auto & view : viewers) view->SetVisible(true);
    viewers[0]->SetBounds(3, 3, Width2, Height2);
    viewers[1]->SetBounds(6 + Width2, 3, Width2, Height2);
    viewers[2]->SetBounds(3, 6 + Height2, Width2, Height2);
    viewers[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);

    if (modeSolo) {

        // Go from single to layout
        xs1 = (double) 3;
        ys1 = (double) 3;
        xs2 = (double) fWidth + xs1;
        ys2 = (double) fHeight + ys1;

        switch (next) {
            case 0:
                xe1 = (double) (3);
                ye1 = (double) (3);
                break;
            case 1:
                xe1 = (double) (5 + Width2);
                ye1 = (double) (3);
                break;
            case 2:
                xe1 = (double) (3);
                ye1 = (double) (5 + Height2);
                break;
            case 3:
                xe1 = (double) (5 + Width2);
                ye1 = (double) (5 + Height2);
                break;
            default:
                break;
        }

        xe2 = (double) (Width2) + xe1;
        ye2 = (double) (Height2) + ye1;

    } else {

        // Go from layout to single
        xe1 = (double) 3;
        ye1 = (double) 3;
        xe2 = (double) fWidth + xe1;
        ye2 = (double) fHeight + ye1;

        switch (next) {
            case 0:
                xs1 = (double) (3);
                ys1 = (double) (3);
                break;
            case 1:
                xs1 = (double) (5 + Width2);
                ys1 = (double) (3);
                break;
            case 2:
                xs1 = (double) (3);
                ys1 = (double) (5 + Height2);
                break;
            case 3:
                xs1 = (double) (5 + Width2);
                ys1 = (double) (5 + Height2);
                break;
            default:
                break;
        }

        xs2 = (double) (Width2) + xs1;
        ys2 = (double) (Height2) + ys1;

    }

    double t0 = (double) SDL_GetTicks() / 1000.0;
    double t1 = t0;
    double T = 0.15;

    while ((t1 - t0) < T) {
        double t = (t1 - t0) / T;
        int x1 = lround(xs1 + t * (xe1 - xs1));
        int y1 = lround(ys1 + t * (ye1 - ys1));
        int x2 = lround(xs2 + t * (xe2 - xs2));
        int y2 = lround(ys2 + t * (ye2 - ys2));
        viewers[next]->SetBounds(x1, y1, x2 - x1, y2 - y1);
        wnd->Paint();
        // Overides moving component
        viewers[next]->Paint();
        // Paint modeless
        int n;
        n = GLWindowManager::GetNbWindow();
        GLWindowManager::RepaintRange(1, n);
        t1 = (double) SDL_GetTicks() / 1000.0;
    }

    modeSolo = !modeSolo;
    SelectViewer(next);

}

void Interface::UpdateViewerPanel() {

    showNormal->SetState(viewers[curViewer]->showNormal);
    showRule->SetState(viewers[curViewer]->showRule);
    showUV->SetState(viewers[curViewer]->showUV);
    showLeak->SetState(viewers[curViewer]->showLeak);
    showHit->SetState(viewers[curViewer]->showHit);
    showVolume->SetState(viewers[curViewer]->showVolume);
    showLine->SetState(viewers[curViewer]->showLine);
    showTexture->SetState(viewers[curViewer]->showTexture);
    showFacetId->SetState(viewers[curViewer]->showFacetId);
    showVertexId->SetState(viewers[curViewer]->showVertexId);
    showIndex->SetState(viewers[curViewer]->showIndex);
}

void Interface::SelectViewer(int s) {

    curViewer = s;
    for (int i = 0; i < MAX_VIEWER; i++) viewers[i]->SetSelected(i == curViewer);
    UpdateViewerPanel();
    if (crossSectionWindow) crossSectionWindow->SetViewer(curViewer);
}

void Interface::Place3DViewer() {

    int sx = m_screenWidth - 205;

    // 3D Viewer ----------------------------------------------
    int fWidth = m_screenWidth - 215;
    int fHeight = m_screenHeight - 27;
    int Width2 = fWidth / 2 - 1;
    int Height2 = fHeight / 2 - 1;

    if (modeSolo) {
        for (auto & view : viewers)
            view->SetVisible(false);
        viewers[curViewer]->SetBounds(3, 3, fWidth, fHeight);
        viewers[curViewer]->SetVisible(true);
    } else {
        for (auto & view : viewers)
            view->SetVisible(true);
        viewers[0]->SetBounds(3, 3, Width2, Height2);
        viewers[1]->SetBounds(6 + Width2, 3, Width2, Height2);
        viewers[2]->SetBounds(3, 6 + Height2, Width2, Height2);
        viewers[3]->SetBounds(6 + Width2, 6 + Height2, Width2, Height2);
    }
}

void Interface::UpdateViewers() {
    for (auto & view : viewers)
        view->UpdateMatrix();
}

void Interface::SetFacetSearchPrg(bool visible, const char *text) {
    static Uint32 lastUpd = 0;
    Uint32 now = SDL_GetTicks();
    if (!visible || (now - lastUpd > 500)) {
        for (auto & view : viewers) {
            view->facetSearchState->SetVisible(visible);
            view->facetSearchState->SetText(text);
        }
        GLWindowManager::Repaint();
        lastUpd = now;
    }
}

void Interface::OneTimeSceneInit_shared_pre() {

    GLToolkit::SetIcon32x32("images/app_icon.png");

    for (int i = 0; i < MAX_VIEWER; i++) {
        viewers[i] = new GeometryViewer(i);
        Add(viewers[i]);
    }
    modeSolo = true;
    //nbSt = 0;



    menu = new GLMenuBar(0);
    wnd->SetMenuBar(menu);
    menu->Add("File");
    menu->GetSubMenu("File")->Add("New, empty geometry", MENU_FILE_NEW);
    menu->GetSubMenu("File")->Add("&Load", MENU_FILE_LOAD, SDLK_o, CTRL_MODIFIER);
    menu->GetSubMenu("File")->Add("Load recent");
    menu->GetSubMenu("File")->Add(nullptr); //separator
    menu->GetSubMenu("File")->Add("&Insert geometry");
    menu->GetSubMenu("File")->GetSubMenu("Insert geometry")->Add("&To current structure", MENU_FILE_INSERTGEO);
    menu->GetSubMenu("File")->GetSubMenu("Insert geometry")->Add("&To new structure", MENU_FILE_INSERTGEO_NEWSTR);
    menu->GetSubMenu("File")->Add(nullptr); //separator
    menu->GetSubMenu("File")->Add("&Save", MENU_FILE_SAVE, SDLK_s, CTRL_MODIFIER);
    menu->GetSubMenu("File")->Add("&Save as", MENU_FILE_SAVEAS);
    menu->GetSubMenu("File")->Add(nullptr); //separator

    menu->GetSubMenu("File")->Add("Export selected facets", MENU_FILE_EXPORT_SELECTION);

    menu->GetSubMenu("File")->Add("Export selected profiles", MENU_FILE_EXPORTPROFILES);

    menu->GetSubMenu("File")->SetIcon(MENU_FILE_SAVE, 83, 24);
    menu->GetSubMenu("File")->SetIcon(MENU_FILE_SAVEAS, 101, 24);
    menu->GetSubMenu("File")->SetIcon(MENU_FILE_LOAD, 65, 24);//65,24
    //menu->GetSubMenu("File")->SetIcon(MENU_FILE_LOADRECENT,83,24);//83,24

    menu->Add("Selection");
    menu->GetSubMenu("Selection")->Add("Smart Select facets...", MENU_SELECTION_SMARTSELECTION, SDLK_s, ALT_MODIFIER);
    menu->GetSubMenu("Selection")->Add(nullptr); // Separator
    menu->GetSubMenu("Selection")->Add("Select All Facets", MENU_FACET_SELECTALL, SDLK_a, CTRL_MODIFIER);
    menu->GetSubMenu("Selection")->Add("Select by Facet Number...", MENU_SELECTION_SELECTFACETNUMBER, SDLK_n,
                                       ALT_MODIFIER);
    menu->GetSubMenu("Selection")->Add("Select Sticking", MENU_FACET_SELECTSTICK);
    menu->GetSubMenu("Selection")->Add("Select Transparent", MENU_FACET_SELECTTRANS);
    menu->GetSubMenu("Selection")->Add("Select 2 sided", MENU_FACET_SELECT2SIDE);
    menu->GetSubMenu("Selection")->Add("Select Texture", MENU_FACET_SELECTTEXT);
    menu->GetSubMenu("Selection")->Add("Select by Texture type...", MENU_SELECTION_TEXTURETYPE);
    menu->GetSubMenu("Selection")->Add("Select Profile", MENU_FACET_SELECTPROF);

    menu->GetSubMenu("Selection")->Add(nullptr); // Separator
    menu->GetSubMenu("Selection")->Add("Select Abs > 0", MENU_FACET_SELECTABS);
    menu->GetSubMenu("Selection")->Add("Select Hit > 0", MENU_FACET_SELECTHITS);
    menu->GetSubMenu("Selection")->Add("Select large with no hits...", MENU_FACET_SELECTNOHITS_AREA);
    menu->GetSubMenu("Selection")->Add("Select by facet result...", MENU_FACET_SELECT_BY_RESULT);
    menu->GetSubMenu("Selection")->Add(nullptr); // Separator

    menu->GetSubMenu("Selection")->Add("Select link facets", MENU_FACET_SELECTDEST);
    menu->GetSubMenu("Selection")->Add("Select teleport facets", MENU_FACET_SELECTTELEPORT);
    menu->GetSubMenu("Selection")->Add("Select non planar facets", MENU_FACET_SELECTNONPLANAR);
    menu->GetSubMenu("Selection")->Add("Select non simple facets", MENU_FACET_SELECTERR);
    //menu->GetSubMenu("Selection")->Add(nullptr); // Separator
    //menu->GetSubMenu("Selection")->Add("Load selection",MENU_FACET_LOADSEL);
    //menu->GetSubMenu("Selection")->Add("Save selection",MENU_FACET_SAVESEL);
    menu->GetSubMenu("Selection")->Add("Invert selection", MENU_FACET_INVERTSEL, SDLK_i, CTRL_MODIFIER);
    menu->GetSubMenu("Selection")->Add(nullptr); // Separator

    menu->GetSubMenu("Selection")->Add("Save / Overwrite selection");
    memorizeSelectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Save / Overwrite selection");
    memorizeSelectionsMenu->Add("Add new...", MENU_SELECTION_ADDNEW, SDLK_w, CTRL_MODIFIER);
    memorizeSelectionsMenu->Add(nullptr); // Separator

    menu->GetSubMenu("Selection")->Add("Select memorized");
    selectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Select memorized");

    menu->GetSubMenu("Selection")->Add("Clear memorized", MENU_SELECTION_CLEARSELECTIONS);
    clearSelectionsMenu = menu->GetSubMenu("Selection")->GetSubMenu("Clear memorized");
    clearSelectionsMenu->Add("Clear All", MENU_SELECTION_CLEARALL);
    clearSelectionsMenu->Add(nullptr); // Separator

    menu->Add("Tools");

    menu->GetSubMenu("Tools")->Add("Formula editor", MENU_TOOLS_FORMULAEDITOR, SDLK_f, ALT_MODIFIER);
    menu->GetSubMenu("Tools")->Add("Convergence Plotter ...", MENU_TOOLS_CONVPLOTTER, SDLK_c, ALT_MODIFIER);
    menu->GetSubMenu("Tools")->Add(nullptr); // Separator
    menu->GetSubMenu("Tools")->Add("Texture Plotter ...", MENU_TOOLS_TEXPLOTTER, SDLK_t, ALT_MODIFIER);
    menu->GetSubMenu("Tools")->Add("Profile Plotter ...", MENU_TOOLS_PROFPLOTTER, SDLK_p, ALT_MODIFIER);
#if defined(MOLFLOW)
    menu->GetSubMenu("Tools")->Add("Histogram Plotter...", MENU_TOOLS_HISTOGRAMPLOTTER);
#endif
    menu->GetSubMenu("Tools")->Add(nullptr); // Separator
    menu->GetSubMenu("Tools")->Add("Texture scaling...", MENU_EDIT_TSCALING, SDLK_d, CTRL_MODIFIER);
    menu->GetSubMenu("Tools")->Add("Particle logger...", MENU_TOOLS_PARTICLELOGGER);
    //menu->GetSubMenu("Tools")->Add("Histogram settings...", MENU_TOOLS_HISTOGRAMSETTINGS, SDLK_t, CTRL_MODIFIER);
    menu->GetSubMenu("Tools")->Add("Global Settings ...", MENU_EDIT_GLOBALSETTINGS);
    menu->GetSubMenu("Tools")->Add(nullptr); // Separator
    menu->GetSubMenu("Tools")->Add("Take screenshot", MENU_TOOLS_SCREENSHOT, SDLK_r, CTRL_MODIFIER);

    menu->GetSubMenu("Tools")->SetIcon(MENU_EDIT_TSCALING, 137, 24);
    menu->GetSubMenu("Tools")->SetIcon(MENU_TOOLS_FORMULAEDITOR, 155, 24);
    menu->GetSubMenu("Tools")->SetIcon(MENU_EDIT_GLOBALSETTINGS, 0, 77);

    menu->Add("Facet");
    menu->GetSubMenu("Facet")->Add("Delete", MENU_FACET_REMOVESEL, SDLK_DELETE, CTRL_MODIFIER);
    menu->GetSubMenu("Facet")->Add("Swap normal", MENU_FACET_SWAPNORMAL, SDLK_n, CTRL_MODIFIER);
    menu->GetSubMenu("Facet")->Add("Shift indices", MENU_FACET_SHIFTVERTEX, SDLK_h, CTRL_MODIFIER);
    menu->GetSubMenu("Facet")->Add("Facet coordinates ...", MENU_FACET_COORDINATES);
    menu->GetSubMenu("Facet")->Add("Move ...", MENU_FACET_MOVE);
    menu->GetSubMenu("Facet")->Add("Scale ...", MENU_FACET_SCALE);
    menu->GetSubMenu("Facet")->Add("Mirror / Project ...", MENU_FACET_MIRROR);
    menu->GetSubMenu("Facet")->Add("Rotate ...", MENU_FACET_ROTATE);
    menu->GetSubMenu("Facet")->Add("Align to ...", MENU_FACET_ALIGN);
    menu->GetSubMenu("Facet")->Add("Extrude ...", MENU_FACET_EXTRUDE);
    menu->GetSubMenu("Facet")->Add("Split ...", MENU_FACET_SPLIT);
    menu->GetSubMenu("Facet")->Add(nullptr);
    menu->GetSubMenu("Facet")->Add("Create shape...", MENU_FACET_CREATESHAPE);
    menu->GetSubMenu("Facet")->Add("Create two facets' ...");
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Difference");
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("Auto (non-zero)",
                                                                                                   MENU_FACET_CREATE_DIFFERENCE_AUTO);
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("First - Second",
                                                                                                   MENU_FACET_CREATE_DIFFERENCE);
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->GetSubMenu("Difference")->Add("Second - First",
                                                                                                   MENU_FACET_CREATE_DIFFERENCE2);
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Union", MENU_FACET_CREATE_UNION);
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("Intersection",
                                                                         MENU_FACET_CREATE_INTERSECTION);
    menu->GetSubMenu("Facet")->GetSubMenu("Create two facets' ...")->Add("XOR", MENU_FACET_CREATE_XOR);
    menu->GetSubMenu("Facet")->Add("Transition between 2", MENU_FACET_LOFT);
    menu->GetSubMenu("Facet")->Add("Build intersection...", MENU_FACET_INTERSECT);
    menu->GetSubMenu("Facet")->Add(nullptr);
    menu->GetSubMenu("Facet")->Add("Collapse ...", MENU_FACET_COLLAPSE);
    menu->GetSubMenu("Facet")->Add("Explode", MENU_FACET_EXPLODE);
    menu->GetSubMenu("Facet")->Add("Revert flipped normals (old geometries)", MENU_FACET_REVERTFLIP);
    menu->GetSubMenu("Facet")->Add("Triangulate", MENU_FACET_TRIANGULATE);

    //menu->GetSubMenu("Facet")->Add("Facet Details ...", MENU_FACET_DETAILS);
    //menu->GetSubMenu("Facet")->Add("Facet Mesh ...",MENU_FACET_MESH);

    //facetMenu = menu->GetSubMenu("Facet");
    //facetMenu->SetEnabled(MENU_FACET_MESH,false);

    menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_COLLAPSE, 173, 24);
    menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_SWAPNORMAL, 191, 24);
    menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_SHIFTVERTEX, 90, 77);
    menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_COORDINATES, 209, 24);
    menu->GetSubMenu("Facet")->SetIcon(MENU_TOOLS_PROFPLOTTER, 227, 24);

    menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_DETAILS, 54, 77);
    //menu->GetSubMenu("Facet")->SetIcon(MENU_FACET_MESH,72,77);
    menu->GetSubMenu("Facet")->SetIcon(MENU_TOOLS_TEXPLOTTER, 108, 77);

    menu->Add("Vertex");
    menu->GetSubMenu("Vertex")->Add("Create Facet from Selected");
    menu->GetSubMenu("Vertex")->GetSubMenu("Create Facet from Selected")->Add("Convex Hull",
                                                                              MENU_VERTEX_CREATE_POLY_CONVEX, SDLK_v,
                                                                              ALT_MODIFIER);
    menu->GetSubMenu("Vertex")->GetSubMenu("Create Facet from Selected")->Add("Keep selection order",
                                                                              MENU_VERTEX_CREATE_POLY_ORDER);
    menu->GetSubMenu("Vertex")->Add("Clear isolated", MENU_VERTEX_CLEAR_ISOLATED);
    menu->GetSubMenu("Vertex")->Add("Remove selected", MENU_VERTEX_REMOVE);
    menu->GetSubMenu("Vertex")->Add("Vertex coordinates...", MENU_VERTEX_COORDINATES);
    menu->GetSubMenu("Vertex")->Add("Move...", MENU_VERTEX_MOVE);
    menu->GetSubMenu("Vertex")->Add("Scale...", MENU_VERTEX_SCALE);
    menu->GetSubMenu("Vertex")->Add("Mirror / Project ...", MENU_VERTEX_MIRROR);
    menu->GetSubMenu("Vertex")->Add("Rotate...", MENU_VERTEX_ROTATE);
    menu->GetSubMenu("Vertex")->Add("Add new...", MENU_VERTEX_ADD);
    menu->GetSubMenu("Vertex")->Add(nullptr); // Separator
    menu->GetSubMenu("Vertex")->Add("Select all vertex", MENU_VERTEX_SELECTALL);
    menu->GetSubMenu("Vertex")->Add("Unselect all vertex", MENU_VERTEX_UNSELECTALL);
    menu->GetSubMenu("Vertex")->Add("Select coplanar vertex (visible on screen)", MENU_VERTEX_SELECT_COPLANAR);
    menu->GetSubMenu("Vertex")->Add("Select isolated vertex", MENU_VERTEX_SELECT_ISOLATED);

    menu->Add("View");

    menu->GetSubMenu("View")->Add("Structure");
    structMenu = menu->GetSubMenu("View")->GetSubMenu("Structure");
    UpdateStructMenu();

    menu->GetSubMenu("View")->Add("Cross section...", MENU_VIEW_CROSSSECTION);
    menu->GetSubMenu("View")->Add("Full Screen", MENU_VIEW_FULLSCREEN);

    menu->GetSubMenu("View")->Add(nullptr); // Separator

    menu->GetSubMenu("View")->Add("Save / Overwrite view");
    memorizeViewsMenu = menu->GetSubMenu("View")->GetSubMenu("Save / Overwrite view");
    memorizeViewsMenu->Add("Add new...", MENU_VIEW_ADDNEW, SDLK_q, CTRL_MODIFIER);
    memorizeViewsMenu->Add(nullptr); // Separator

    menu->GetSubMenu("View")->Add("Select memorized");
    viewsMenu = menu->GetSubMenu("View")->GetSubMenu("Select memorized");

    menu->GetSubMenu("View")->Add("Clear memorized", MENU_VIEW_CLEARVIEWS);
    clearViewsMenu = menu->GetSubMenu("View")->GetSubMenu("Clear memorized");
    clearViewsMenu->Add("Clear All", MENU_VIEW_CLEARALL);

    menu->GetSubMenu("View")->SetIcon(MENU_VIEW_FULLSCREEN, 18, 77);

    menu->Add("Test");
    menu->GetSubMenu("Test")->Add("Pipe (L/R=0.0001)", MENU_TEST_PIPE0001);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=1)", MENU_TEST_PIPE1);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=10)", MENU_TEST_PIPE10);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=100)", MENU_TEST_PIPE100);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=1000)", MENU_TEST_PIPE1000);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=10000)", MENU_TEST_PIPE10000);
    menu->GetSubMenu("Test")->Add("Pipe (L/R=N)", MENU_TEST_PIPEN);
    //Quick test pipe
    menu->GetSubMenu("Test")->Add(nullptr);
    menu->GetSubMenu("Test")->Add("Quick Pipe", MENU_QUICKPIPE, SDLK_q, ALT_MODIFIER);

    menu->GetSubMenu("Test")->Add(nullptr);
    menu->GetSubMenu("Test")->Add("Triangulate Geometry", MENU_TRIANGULATE);
    menu->GetSubMenu("Test")->Add("Analyze Geometry", MENU_ANALYZE);
    menu->GetSubMenu("Test")->Add("Compare Results", MENU_CMP_RES);

    menu->GetSubMenu("Test")->Add(nullptr);
    menu->GetSubMenu("Test")->Add("ImGui Menu", MENU_IMGUI_MENU);
    menu->GetSubMenu("Test")->Add("ImGui Global Settings", MENU_IMGUI_GLOB);
#ifdef DEBUG
    menu->GetSubMenu("Test")->Add("ImGui Sidebar", MENU_IMGUI_SIDE);
#endif
    menu->GetSubMenu("Test")->Add("ImGui Test Suite", MENU_IMGUI);

    geomNumber = new GLTextField(0, nullptr);
    geomNumber->SetEditable(false);
    Add(geomNumber);

    togglePanel = new GLTitledPanel("3D Viewer settings");
    togglePanel->SetClosable(true);
    Add(togglePanel);

    showNormal = new GLToggle(0, "Normals");
    togglePanel->Add(showNormal);

    showRule = new GLToggle(0, "Axes");
    togglePanel->Add(showRule);

    showUV = new GLToggle(0, "\201,\202");
    togglePanel->Add(showUV);

    showLeak = new GLToggle(0, "Leaks");
    togglePanel->Add(showLeak);

    showHit = new GLToggle(0, "Hits");
    togglePanel->Add(showHit);

    showLine = new GLToggle(0, "Lines");
    togglePanel->Add(showLine);

    showVolume = new GLToggle(0, "Volume");
    togglePanel->Add(showVolume);

    showTexture = new GLToggle(0, "Texture");
    togglePanel->Add(showTexture);

    showFacetId = new GLToggle(0, "FacetIDs");
    togglePanel->Add(showFacetId);

    showIndex = new GLToggle(0, "Indices");
    togglePanel->Add(showIndex);

    showVertexId = new GLToggle(0, "VertexIDs");
    togglePanel->Add(showVertexId);

    simuPanel = new GLTitledPanel("Simulation");
    simuPanel->SetClosable(true);
    Add(simuPanel);

    globalSettingsBtn = new GLButton(0, "<< Sim");
    simuPanel->Add(globalSettingsBtn);

    startSimu = new GLButton(0, "Start/Stop");
    simuPanel->Add(startSimu);

    resetSimu = new GLButton(0, "Reset");
    simuPanel->Add(resetSimu);

    autoFrameMoveToggle = new GLToggle(0, "Auto update scene");
    autoFrameMoveToggle->SetState(autoFrameMove);
    simuPanel->Add(autoFrameMoveToggle);

    forceFrameMoveButton = new GLButton(0, "Update");
    forceFrameMoveButton->SetEnabled(!autoFrameMove);
    simuPanel->Add(forceFrameMoveButton);

    hitLabel = new GLLabel("Hits");
    simuPanel->Add(hitLabel);

    hitNumber = new GLTextField(0, nullptr);
    hitNumber->SetEditable(false);
    simuPanel->Add(hitNumber);

    desLabel = new GLLabel("Des.");
    simuPanel->Add(desLabel);

    desNumber = new GLTextField(0, nullptr);
    desNumber->SetEditable(false);
    simuPanel->Add(desNumber);

    leakLabel = new GLLabel("Leaks");
    simuPanel->Add(leakLabel);

    leakNumber = new GLTextField(0, nullptr);
    leakNumber->SetEditable(false);
    simuPanel->Add(leakNumber);

    sTimeLabel = new GLLabel("Time");
    simuPanel->Add(sTimeLabel);

    sTime = new GLTextField(0, nullptr);
    sTime->SetEditable(false);
    simuPanel->Add(sTime);

    facetPanel = new GLTitledPanel("Selected Facet");
    facetPanel->SetClosable(true);
    Add(facetPanel);

    facetSideLabel = new GLLabel("Sides:");
    facetPanel->Add(facetSideLabel);

    facetSideType = new GLCombo(0);
    facetSideType->SetSize(2);
    facetSideType->SetValueAt(0, "1 Sided");
    facetSideType->SetValueAt(1, "2 Sided");
    facetPanel->Add(facetSideType);

    facetTLabel = new GLLabel("Opacity:");
    facetPanel->Add(facetTLabel);
    facetOpacity = new GLTextField(0, nullptr);
    facetPanel->Add(facetOpacity);

    facetAreaLabel = new GLLabel("Area (cm\262):");
    facetPanel->Add(facetAreaLabel);
    facetAreaText = new GLTextField(0, nullptr);
    facetPanel->Add(facetAreaText);

    facetDetailsBtn = new GLButton(0, "Details...");
    facetPanel->Add(facetDetailsBtn);

    facetCoordBtn = new GLButton(0, "Coord.");
    facetPanel->Add(facetCoordBtn);

    facetApplyBtn = new GLButton(0, "Apply");
    facetApplyBtn->SetEnabled(false);
    facetPanel->Add(facetApplyBtn);
}

void Interface::OneTimeSceneInit_shared_post() {
    menu->Add("About");
    menu->GetSubMenu("About")->Add("License", MENU_ABOUT);
    menu->GetSubMenu("About")->Add("Check for updates...", MENU_UPDATE);

    ClearFacetParams();
    LoadConfig();
    UpdateRecentMenu();
    UpdateViewerPanel();
    PlaceComponents();
    CheckNeedsTexture();

    try {
        worker.InitSimProc();
        //worker.SetProcNumber(nbProc);
    }
    catch (const std::exception &e) {
        char errMsg[512];
        sprintf(errMsg, "Failed to start working sub-process(es), simulation not available\n%s", e.what());
        GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
    }


    EmptyGeometry();

    appUpdater = new AppUpdater(appName, appVersionId, "updater_config.xml");
    int answer = appUpdater->RequestUpdateCheck();
    if (answer == ANSWER_ASKNOW) {
        updateCheckDialog = new UpdateCheckDialog(appName, appUpdater);
        updateCheckDialog->SetVisible(true);
        wereEvents = true;
    }
}

int Interface::RestoreDeviceObjects_shared() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    interfGeom->RestoreDeviceObjects();
    //worker.Update(0.0f);

    // Restore dialog which are not displayed
    // Those which are displayed are invalidated by the window manager
    RVALIDATE_DLG(formulaEditor);
    RVALIDATE_DLG(collapseSettings);
    RVALIDATE_DLG(histogramSettings);
    RVALIDATE_DLG(histogramPlotter);
    RVALIDATE_DLG(moveVertex);
    RVALIDATE_DLG(scaleVertex);
    RVALIDATE_DLG(scaleFacet);
    RVALIDATE_DLG(selectDialog);
    RVALIDATE_DLG(selectFacetByResult);
    RVALIDATE_DLG(selectTextureType);
    RVALIDATE_DLG(moveFacet);
    RVALIDATE_DLG(createShape);
    RVALIDATE_DLG(extrudeFacet);
    RVALIDATE_DLG(mirrorFacet);
    RVALIDATE_DLG(mirrorVertex);
    RVALIDATE_DLG(splitFacet);
    RVALIDATE_DLG(buildIntersection);
    RVALIDATE_DLG(rotateFacet);
    RVALIDATE_DLG(rotateVertex);
    RVALIDATE_DLG(alignFacet);
    RVALIDATE_DLG(addVertex);
    RVALIDATE_DLG(facetCoordinates);
    RVALIDATE_DLG(vertexCoordinates);
    RVALIDATE_DLG(particleLogger);
    RVALIDATE_DLG(convergencePlotter);

    RVALIDATE_DLG(updateCheckDialog);
    RVALIDATE_DLG(updateFoundDialog);
    RVALIDATE_DLG(updateLogWindow);
    RVALIDATE_DLG(manualUpdate);

    UpdateTitle();

    return GL_OK;
}

int Interface::InvalidateDeviceObjects_shared() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    interfGeom->InvalidateDeviceObjects();
    //worker.Update(0.0f);

    // Restore dialog which are not displayed
    // Those which are displayed are invalidated by the window manager
    IVALIDATE_DLG(formulaEditor);
    IVALIDATE_DLG(collapseSettings);
    IVALIDATE_DLG(histogramSettings);
    IVALIDATE_DLG(histogramPlotter);
    IVALIDATE_DLG(moveVertex);
    IVALIDATE_DLG(scaleVertex);
    IVALIDATE_DLG(scaleFacet);
    IVALIDATE_DLG(selectDialog);
    IVALIDATE_DLG(selectTextureType);
    IVALIDATE_DLG(selectFacetByResult);
    IVALIDATE_DLG(moveFacet);
    IVALIDATE_DLG(createShape);
    IVALIDATE_DLG(extrudeFacet);
    IVALIDATE_DLG(mirrorFacet);
    IVALIDATE_DLG(mirrorVertex);
    IVALIDATE_DLG(splitFacet);
    IVALIDATE_DLG(buildIntersection);
    IVALIDATE_DLG(rotateFacet);
    IVALIDATE_DLG(rotateFacet);
    IVALIDATE_DLG(alignFacet);
    IVALIDATE_DLG(addVertex);
    IVALIDATE_DLG(facetCoordinates);
    IVALIDATE_DLG(vertexCoordinates);
    IVALIDATE_DLG(particleLogger);
    IVALIDATE_DLG(convergencePlotter);

    IVALIDATE_DLG(updateCheckDialog);
    IVALIDATE_DLG(updateFoundDialog);
    IVALIDATE_DLG(updateLogWindow);
    IVALIDATE_DLG(manualUpdate);

    UpdateTitle();

    return GL_OK;
}

bool Interface::ProcessMessage_shared(GLComponent *src, int message) {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char tmp[128];

    switch (message) {

        //MENU --------------------------------------------------------------------
        case MSG_MENU:
            switch (src->GetId()) {
                case MENU_FILE_NEW:
                    if (AskToSave()) {
                        if (worker.IsRunning()) worker.Stop_Public();
                        EmptyGeometry();
                    }
                    return true;
                case MENU_FILE_LOAD:
                    if (AskToSave()) {
                        if (worker.IsRunning()) worker.Stop_Public();
                        LoadFile("");
                    }
                    return true;
                case MENU_FILE_INSERTGEO:
                    if (interfGeom->IsLoaded()) {
                        if (worker.IsRunning()) worker.Stop_Public();
                        InsertGeometry(false,"");
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FILE_INSERTGEO_NEWSTR:
                    if (interfGeom->IsLoaded()) {
                        if (worker.IsRunning()) worker.Stop_Public();
                        InsertGeometry(true, "");
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FILE_SAVEAS:
                    if (interfGeom->IsLoaded()) {
                        SaveFileAs();
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FILE_EXPORT_SELECTION:
                    ExportSelection();
                    return true;
                case MENU_FILE_SAVE:
                    if (interfGeom->IsLoaded()) SaveFile();
                    else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FILE_EXIT:
                    if (AskToSave()) RequestExit();
                    return true;

                    /*case MENU_EDIT_ADDFORMULA:
                        if (!formulaSettings) formulaSettings = new FormulaSettings();
                        formulaSettings->Update(nullptr, -1);
                        formulaSettings->SetVisible(true);
                        return true;*/

                case MENU_TOOLS_FORMULAEDITOR:
                    if (!interfGeom->IsLoaded()) {
                        GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                        return true;
                    }
                    if (!formulaEditor || !formulaEditor->IsVisible()) {
                        SAFE_DELETE(formulaEditor);
                        formulaEditor = new FormulaEditor(&worker, appFormulas);
                        formulaEditor->Refresh();
                        // Load values on init
                        appFormulas->EvaluateFormulas(worker.globalStatCache.globalHits.nbDesorbed);
                        formulaEditor->UpdateValues();
                        // ---
                        formulaEditor->SetVisible(true);
                    }
                    break;
                case MENU_TOOLS_HISTOGRAMSETTINGS:
                    if (!histogramSettings || !histogramSettings->IsVisible()) {
                        SAFE_DELETE(histogramSettings);
                        histogramSettings = new HistogramSettings(interfGeom, &worker);
                    }
                    histogramSettings->Refresh(interfGeom->GetSelectedFacets());
                    histogramSettings->SetVisible(true);
                    return true;
                case MENU_TOOLS_HISTOGRAMPLOTTER:
                    if (!histogramPlotter || !histogramPlotter->IsVisible()) {
                        SAFE_DELETE(histogramPlotter);
                        histogramPlotter = new HistogramPlotter(&worker);
                    }
                    histogramPlotter->Refresh();
                    histogramPlotter->SetVisible(true);
                    return true;
                case MENU_TOOLS_CONVPLOTTER:
                    if (!convergencePlotter)
                        convergencePlotter = new ConvergencePlotter(&worker, appFormulas);
                    else{
                        if(!convergencePlotter->IsVisible()) {
                            auto *newConv = new ConvergencePlotter(*convergencePlotter);
                            //newConv->SetViews(convergencePlotter->GetViews());
                            SAFE_DELETE(convergencePlotter);
                            convergencePlotter = newConv;
                        }
                    }
                    convergencePlotter->Display(&worker);
                    return true;
                case MENU_TOOLS_PARTICLELOGGER:
                    if (!particleLogger || !particleLogger->IsVisible()) {
                        SAFE_DELETE(particleLogger);
                        particleLogger = new ParticleLogger(interfGeom, &worker);
                    }
                    particleLogger->UpdateStatus();
                    particleLogger->SetVisible(true);
                    return true;

                case MENU_TOOLS_SCREENSHOT: {
                    std::ostringstream tmp_ss;

                    char buf[80];
                    time_t now = time(nullptr);
                    struct tm tstruct = *localtime(&now);
                    // Visit http://en.cppreference.com/w/cpp/chrono/c/strftime
                    // for more information about date/time format
                    strftime(buf, sizeof(buf), "%Y_%m_%d__%H_%M_%S", &tstruct);

                    tmp_ss << buf << "_" << worker.GetCurrentShortFileName();
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
                    viewers[curViewer]->GetBounds(&x, &y, &width, &height);

                    int leftMargin = 4; //Left bewel
                    int rightMargin = 0;
                    int topMargin = 0;
                    int bottomMargin = 28; //Toolbar

                    viewers[curViewer]->RequestScreenshot(tmp_ss.str(), leftMargin, topMargin,
                                                         width - leftMargin - rightMargin,
                                                         height - topMargin - bottomMargin);
                    return true;
                }
                case MENU_FACET_COLLAPSE:
                    if (interfGeom->IsLoaded()) {
                        DisplayCollapseDialog();
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FACET_SWAPNORMAL:
                    if (AskToReset()) {
                        interfGeom->SwapNormal();
                        // Send to sub process
                        worker.MarkToReload();
                    }
                    return true;
                case MENU_FACET_REVERTFLIP:
                    if (AskToReset()) {
                        interfGeom->RevertFlippedNormals();
                        // Send to sub process
                        worker.MarkToReload();
                    }
                    return true;
                case MENU_FACET_EXTRUDE:
                    if (!extrudeFacet || !extrudeFacet->IsVisible()) {
                        SAFE_DELETE(extrudeFacet);
                        extrudeFacet = new ExtrudeFacet(interfGeom, &worker);
                    }
                    extrudeFacet->SetVisible(true);
                    return true;

                case MENU_FACET_SHIFTVERTEX:
                    if (AskToReset()) {
                        interfGeom->ShiftVertex();
                        // Send to sub process
                        worker.MarkToReload();
                    }
                    return true;
                case MENU_FACET_COORDINATES:

                    if (!facetCoordinates) facetCoordinates = new FacetCoordinates();
                    facetCoordinates->Display(&worker);
                    return true;
                case MENU_FACET_MOVE:
                    if (!moveFacet || !moveFacet->IsVisible()) {
                        SAFE_DELETE(moveFacet);
                        moveFacet = new MoveFacet(interfGeom, &worker);
                    }
                    moveFacet->SetVisible(true);
                    return true;
                case MENU_FACET_SCALE:
                    if (interfGeom->IsLoaded()) {
                        if (!scaleFacet) scaleFacet = new ScaleFacet(interfGeom, &worker);

                        scaleFacet->SetVisible(true);

                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_FACET_MIRROR:
                    if (!mirrorFacet) mirrorFacet = new MirrorFacet(interfGeom, &worker);
                    mirrorFacet->SetVisible(true);
                    return true;
                case MENU_FACET_SPLIT:
                    if (!splitFacet || !splitFacet->IsVisible()) {
                        SAFE_DELETE(splitFacet);
                        splitFacet = new SplitFacet(interfGeom, &worker);
                        splitFacet->SetVisible(true);
                    }
                    return true;
                case MENU_FACET_ROTATE:
                    if (!rotateFacet) rotateFacet = new RotateFacet(interfGeom, &worker);
                    rotateFacet->SetVisible(true);
                    return true;
                case MENU_FACET_ALIGN:
                    if (!alignFacet) alignFacet = new AlignFacet(interfGeom, &worker);
                    alignFacet->MemorizeSelection();
                    alignFacet->SetVisible(true);
                    return true;

                case MENU_FACET_EXPLODE:
                    if (GLMessageBox::Display("Explode selected facet?", "Question", GLDLG_OK | GLDLG_CANCEL,
                                              GLDLG_ICONINFO) == GLDLG_OK) {
                        if (AskToReset()) {
                            int err;
                            try {
                                err = interfGeom->ExplodeSelected();
                            }
                            catch (const std::exception &e) {
                                GLMessageBox::Display(e.what(), "Error exploding", GLDLG_OK, GLDLG_ICONERROR);
                            }
                            if (err == -1) {
                                GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
                            } else if (err == -2) {
                                GLMessageBox::Display(
                                        "All selected facets must have a mesh with boudary correction enabled", "Error",
                                        GLDLG_OK, GLDLG_ICONERROR);
                            } else if (err == 0) {

                                UpdateModelParams();
                                UpdateFacetParams(true);
                                // Send to sub process
                                worker.MarkToReload();
                            }
                        }
                    }
                    return true;
                case MENU_FACET_CREATESHAPE:
                    if (!createShape) createShape = new CreateShape(interfGeom, &worker);
                    createShape->SetVisible(true);
                    return true;
                case MENU_SELECTION_SMARTSELECTION:
                    if (!smartSelection) smartSelection = new SmartSelection(worker.GetGeometry(), &worker);
                    smartSelection->SetVisible(true);
                    return true;
                case MENU_FACET_SELECTALL:
                    interfGeom->SelectAll();
                    UpdateFacetParams(true);
                    return true;
                case MENU_FACET_SELECTTRANS:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (
#if defined(MOLFLOW)
!interfGeom->GetFacet(i)->sh.opacityParam.empty() ||
#endif
(interfGeom->GetFacet(i)->sh.opacity != 1.0 && interfGeom->GetFacet(i)->sh.opacity != 2.0))
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                case MENU_FACET_SELECT2SIDE:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (interfGeom->GetFacet(i)->sh.is2sided)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                case MENU_FACET_SELECTTEXT:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (interfGeom->GetFacet(i)->sh.isTextured)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                case MENU_FACET_SELECTPROF:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (interfGeom->GetFacet(i)->sh.isProfile)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTNONPLANAR: {
                    sprintf(tmp, "%g", planarityThreshold);
                    //sprintf(title,"Pipe L/R = %g",L/R);
                    char *input = GLInputBox::GetInput(tmp, "Planarity larger than:", "Select non planar facets");
                    if (!input) return true;
                    if (!sscanf(input, "%lf", &planarityThreshold)) {
                        GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        return true;
                    }
                    interfGeom->UnselectAll();
                    std::vector<size_t> nonPlanarFacetids = interfGeom->GetNonPlanarFacetIds(planarityThreshold);
                    for (const auto &i : nonPlanarFacetids)
                        interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                }
                case MENU_FACET_SELECTERR:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)

                        if (interfGeom->GetFacet(i)->nonSimple)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTDEST:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)

                        if (interfGeom->GetFacet(i)->sh.superDest != 0)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTTELEPORT:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)

                        if (interfGeom->GetFacet(i)->sh.teleportDest != 0)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTABS:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
                        if (interfGeom->GetFacet(i)->facetHitCache.nbAbsEquiv > 0)
                            interfGeom->SelectFacet(i);
                    }
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTHITS:
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (interfGeom->GetFacet(i)->facetHitCache.nbMCHit > 0)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;

                case MENU_FACET_SELECTNOHITS_AREA: {
                    sprintf(tmp, "%g", largeAreaThreshold);
                    //sprintf(title,"Pipe L/R = %g",L/R);
                    char *input = GLInputBox::GetInput(tmp, "Min.area (cm\262)", "Select large facets without hits");
                    if (!input) return true;
                    if ((sscanf(input, "%lf", &largeAreaThreshold) <= 0) || (largeAreaThreshold <= 0.0)) {
                        GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        return true;
                    }
                    interfGeom->UnselectAll();
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        if (interfGeom->GetFacet(i)->facetHitCache.nbMCHit == 0 &&
                            interfGeom->GetFacet(i)->sh.area >= largeAreaThreshold)
                            interfGeom->SelectFacet(i);
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                }
                case MENU_FACET_SELECT_BY_RESULT:
                    if (!selectFacetByResult) selectFacetByResult = new SelectFacetByResult(&worker);
                    selectFacetByResult->SetVisible(true);
                    return true;
                case MENU_FACET_INVERTSEL:
                    for (int i = 0; i < interfGeom->GetNbFacet(); i++)
                        interfGeom->GetFacet(i)->selected = !interfGeom->GetFacet(i)->selected;
                    interfGeom->UpdateSelection();
                    UpdateFacetParams(true);
                    return true;
                case MENU_SELECTION_SELECTFACETNUMBER:
                    if (!selectDialog) selectDialog = new SelectDialog(worker.GetGeometry());
                    selectDialog->SetVisible(true);
                    return true;
                case MENU_SELECTION_TEXTURETYPE:
                    if (!selectTextureType) selectTextureType = new SelectTextureType(&worker);
                    selectTextureType->SetVisible(true);
                    return true;
                case MENU_FACET_SAVESEL:
                    SaveSelection();
                    return true;
                case MENU_FACET_LOADSEL:
                    LoadSelection();
                    return true;
                case MENU_SELECTION_ADDNEW: {
                    std::stringstream tmp_ss;
                    tmp_ss << "Selection #" << (selections.size() + 1);
                    char *selectionName = GLInputBox::GetInput(tmp_ss.str().c_str(), "Selection name",
                                                               "Enter selection name");
                    if (selectionName)
                        AddSelection(selectionName);
                    return true;
                }
                case MENU_SELECTION_CLEARALL:
                    if (GLMessageBox::Display("Clear all selections ?", "Question", GLDLG_OK | GLDLG_CANCEL,
                                              GLDLG_ICONINFO) == GLDLG_OK) {
                        ClearAllSelections();
                    }
                    return true;
                case MENU_VERTEX_UNSELECTALL:
                    interfGeom->UnselectAllVertex();
                    return true;
                case MENU_VERTEX_SELECTALL:
                    interfGeom->SelectAllVertex();
                    return true;
                case MENU_VERTEX_SELECT_ISOLATED:
                    interfGeom->SelectIsolatedVertices();
                    return true;
                case MENU_VERTEX_CLEAR_ISOLATED:
                    interfGeom->DeleteIsolatedVertices(false);
                    UpdateModelParams();
                    if (facetCoordinates) facetCoordinates->UpdateFromSelection();
                    if (vertexCoordinates) vertexCoordinates->Update();
                    interfGeom->BuildGLList();
                    return true;
                case MENU_VERTEX_CREATE_POLY_CONVEX:
                    if (AskToReset()) {
                        try {
                            interfGeom->CreatePolyFromVertices_Convex();
                        }
                        catch (const std::exception &e) {
                            GLMessageBox::Display(e.what(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        worker.MarkToReload();
                    }
                    return true;
                case MENU_VERTEX_CREATE_POLY_ORDER:
                    if (AskToReset()) {
                        try {
                            interfGeom->CreatePolyFromVertices_Order();
                        }
                        catch (const std::exception &e) {
                            GLMessageBox::Display(e.what(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
                        }
                        worker.MarkToReload();
                    }
                    return true;
                case MENU_FACET_CREATE_DIFFERENCE:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 0);
                    return true;
                case MENU_FACET_CREATE_DIFFERENCE2:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 1);
                    return true;
                case MENU_FACET_CREATE_DIFFERENCE_AUTO:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Difference, 2);
                    return true;
                case MENU_FACET_CREATE_UNION:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Union);
                    return true;
                case MENU_FACET_CREATE_INTERSECTION:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Intersection);
                    return true;
                case MENU_FACET_CREATE_XOR:
                    CreateOfTwoFacets(Clipper2Lib::ClipType::Xor);
                    return true;
                case MENU_FACET_LOFT:
                    if (interfGeom->GetNbSelectedFacets() != 2) {
                        GLMessageBox::Display("Select exactly 2 facets", "Can't create loft", GLDLG_OK,
                                              GLDLG_ICONERROR);
                        return true;
                    }
                    if (AskToReset()) {
                        interfGeom->CreateLoft();
                    }
                    worker.MarkToReload();
                    UpdateModelParams();
                    UpdateFacetlistSelected();
                    UpdateViewers();
                    return true;
                case MENU_FACET_INTERSECT:
                    if (!buildIntersection || !buildIntersection->IsVisible()) {
                        SAFE_DELETE(buildIntersection);
                        buildIntersection = new BuildIntersection(interfGeom, &worker);
                        buildIntersection->SetVisible(true);
                    }
                    return true;
                case MENU_FACET_TRIANGULATE: {
                    auto selectedFacets = interfGeom->GetSelectedFacets();
                    if (selectedFacets.empty()) return true;
                    int rep = GLMessageBox::Display("Triangulation can't be undone. Are you sure?", "Geometry change",
                                                    GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
                    if (rep == GLDLG_OK) {
                        if (AskToReset()) {
                            auto prg = GLProgress_GUI("Triangulating", "Triangulating");
                            prg.SetVisible(true);
                            GeometryTools::PolygonsToTriangles(interfGeom, selectedFacets, prg);
                        }
                    }
                    RefreshPlotterCombos();
                    if (vertexCoordinates) vertexCoordinates->Update();
                    if (facetCoordinates) facetCoordinates->UpdateFromSelection();
                    // Send to sub process
                    worker.MarkToReload();
                    UpdateModelParams();
                    UpdateViewers();
                    return true;
                }
                case MENU_VERTEX_SELECT_COPLANAR:
                    char *input;
                    if (interfGeom->IsLoaded()) {
                        if (interfGeom->GetNbSelectedVertex() != 3) {
                            GLMessageBox::Display("Select exactly 3 vertices", "Can't define plane", GLDLG_OK,
                                                  GLDLG_ICONERROR);
                            return true;
                        }
                        sprintf(tmp, "%g", coplanarityTolerance);
                        //sprintf(title,"Pipe L/R = %g",L/R);
                        input = GLInputBox::GetInput(tmp, "Tolerance (cm)", "Select coplanar vertices");
                        if (!input) return true;
                        if ((sscanf(input, "%lf", &coplanarityTolerance) <= 0) || (coplanarityTolerance <= 0.0)) {
                            GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
                            return true;
                        }
                        try { viewers[curViewer]->SelectCoplanar(coplanarityTolerance); }
                        catch (const std::exception &e) {
                            GLMessageBox::Display(e.what(), "Error selecting coplanar vertices", GLDLG_OK,
                                                  GLDLG_ICONERROR);
                        }
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_VERTEX_MOVE:
                    if (interfGeom->IsLoaded()) {
                        if (!moveVertex) moveVertex = new MoveVertex(interfGeom, &worker);

                        moveVertex->SetVisible(true);

                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_VERTEX_SCALE:
                    if (interfGeom->IsLoaded()) {
                        if (!scaleVertex) scaleVertex = new ScaleVertex(interfGeom, &worker);
                        scaleVertex->SetVisible(true);
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;
                case MENU_VERTEX_MIRROR:
                    if (!mirrorVertex) mirrorVertex = new MirrorVertex(interfGeom, &worker);
                    mirrorVertex->SetVisible(true);
                    return true;
                case MENU_VERTEX_ROTATE:
                    if (!rotateVertex) rotateVertex = new RotateVertex(interfGeom, &worker);
                    rotateVertex->SetVisible(true);
                    return true;

                case MENU_VERTEX_COORDINATES:

                    if (!vertexCoordinates) vertexCoordinates = new VertexCoordinates();
                    vertexCoordinates->Display(&worker);
                    return true;

                case MENU_VERTEX_ADD:
                    if (interfGeom->IsLoaded()) {
                        if (!addVertex) addVertex = new AddVertex(interfGeom, &worker);
                        addVertex->SetVisible(true);
                    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
                    return true;

                case MENU_VIEW_FULLSCREEN:
                    ToggleFullscreen();
                    menu->GetSubMenu("View")->SetCheck(MENU_VIEW_FULLSCREEN, !m_bWindowed);
                    return true;

                case MENU_VIEW_ADDNEW:
                    AddView();
                    return true;
                    return true;
                case MENU_VIEW_CLEARALL:
                    if (GLMessageBox::Display("Clear all views ?", "Question", GLDLG_OK | GLDLG_CANCEL,
                                              GLDLG_ICONINFO) == GLDLG_OK) {
                        ClearAllViews();
                    }
                    return true;
                case MENU_TEST_PIPE0001:
                    if (AskToSave()) BuildPipe(0.0001, 0);
                    return true;
                case MENU_TEST_PIPE1:
                    if (AskToSave()) BuildPipe(1.0, 0);
                    return true;
                case MENU_TEST_PIPE10:
                    if (AskToSave()) BuildPipe(10.0, 0);
                    return true;
                case MENU_TEST_PIPE100:
                    if (AskToSave()) BuildPipe(100.0, 0);
                    return true;
                case MENU_TEST_PIPE1000:
                    if (AskToSave()) BuildPipe(1000.0, 0);
                    return true;
                case MENU_TEST_PIPE10000:
                    if (AskToSave()) BuildPipe(10000.0, 0);
                    return true;
                case MENU_TEST_PIPEN: {
                    sprintf(tmp, "100");
                    //sprintf(title,"Pipe L/R = %g",L/R);
                    char *chRatio = GLInputBox::GetInput(tmp, "L/R Ratio", "Build Pipe");
                    if (!chRatio) return false;
                    double ratio = 0.0;
                    if ((sscanf(chRatio, "%lf", &ratio) <= 0) || (ratio <= 0.0)) {
                        GLMessageBox::Display("Invalid number", "Error", GLDLG_OK, GLDLG_ICONERROR);
                        return false;
                    }

                    if (AskToSave()) BuildPipe(ratio, 0);
                    return true;
                }
                case MENU_QUICKPIPE:
                    if (AskToSave()) BuildPipe(5.0, 5);
                    return true;
                case MENU_TRIANGULATE:
                    if (AskToSave()) {
                        auto prg = GLProgress_GUI("Triangulating", "Triangulating");
                        prg.SetVisible(true);
                        GeometryTools::PolygonsToTriangles(this->worker.GetGeometry(),prg);
                        this->worker.MarkToReload();
                    }
                    return true;
                case MENU_ANALYZE:
                    GeometryTools::AnalyzeGeometry(this->worker.GetGeometry());
                    return true;
                case MENU_CMP_RES: {
                    const std::string fileName = NFD_OpenFile_Cpp("syn7z", "");
                    const std::string fileName_rhs = NFD_OpenFile_Cpp("syn7z", "");

                    if (fileName.empty() || fileName_rhs.empty()) {
                        return false;
                    }

                    std::string fileName_out = NFD_SaveFile_Cpp("txt", "");
                    if (fileName_out.empty()) {
                        fileName_out = "myCmp.txt";
                    }

                    worker.GetGeometry()->CompareXML_simustate(fileName, fileName_rhs, fileName_out, 1e-3);
                    return true;
                }
                case MENU_ABOUT: {
                    std::ostringstream aboutText;
                    aboutText << "Program:    " << appName << " " << appVersionName << " (" << appVersionId << ")";
                    aboutText << R"(
Authors:     Roberto KERSEVAN / Marton ADY / Pascal BAEHR / Jean-Luc PONS
Copyright:   CERN / E.S.R.F.   (2024)
Website:    https://cern.ch/molflow

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

Full license text: https://www.gnu.org/licenses/old-licenses/gpl-2.0.en.html
)";
                    GLMessageBox::Display(aboutText.str().c_str(), "About", GLDLG_OK, GLDLG_ICONINFO);
                    return true;
                }
                case MENU_UPDATE: {
                    if(!manualUpdate) {
                        if (!updateLogWindow) {
                            updateLogWindow = new UpdateLogWindow(this);
                        }
                        manualUpdate = new ManualUpdateCheckDialog(appName, appVersionName, appUpdater, updateLogWindow, updateFoundDialog);
                    }
                    manualUpdate->Refresh();
                    manualUpdate->SetVisible(true);
                    return true;
                }
                case MENU_IMGUI: {
                    if (!imWnd) {
                        imWnd = new ImguiWindow(this);
                        imWnd->init();
                    }
                    if (!imWnd->show_main_hub) {
                        imWnd->ToggleMainHub(); 
                    }
                    return true;
                }
                case MENU_IMGUI_GLOB: {
                    if(!imWnd) {
                        imWnd = new ImguiWindow(this);
                        imWnd->init();
                    }
                    if (!imWnd->globalSet.IsVisible()) {
                        imWnd->globalSet.Show();
                    }
                    return true;
                }
#ifdef DEBUG
                case MENU_IMGUI_SIDE: {
                    if(!imWnd) {
                        imWnd = new ImguiWindow(this);
                        imWnd->init();
                    }
                    if (!imWnd->show_app_sidebar) {
                        imWnd->ToggleSimSidebar();
                    }
                    return true;
                }
#endif
                case MENU_IMGUI_MENU: {
                    if(!imWnd) {
                        imWnd = new ImguiWindow(this);
                        imWnd->init();
                    }
                    if (!imWnd->show_app_main_menu_bar) {
                        imWnd->ToggleMainMenu();
                    }
                    return true;
                }
            }
            // Load recent menu
            if (src->GetId() >= MENU_FILE_LOADRECENT && src->GetId() < MENU_FILE_LOADRECENT + recentsList.size()) {
                if (AskToSave()) {
                    if (worker.IsRunning()) worker.Stop_Public();
                    LoadFile(recentsList[src->GetId() - MENU_FILE_LOADRECENT]);
                }
                return true;
            }

                // Show structure menu
            else if (src->GetId() >= MENU_VIEW_STRUCTURE &&
                     src->GetId() <= MENU_VIEW_STRUCTURE + interfGeom->GetNbStructure()) {
                interfGeom->viewStruct = src->GetId() - MENU_VIEW_STRUCTURE - 1;
                if (src->GetId() > MENU_VIEW_STRUCTURE) interfGeom->UnselectAll();
                UpdateStructMenu();
                return true;
            } else if (src->GetId() == MENU_VIEW_NEWSTRUCT) {
                AddStruct();
                UpdateStructMenu();
                return true;
            } else if (src->GetId() == MENU_VIEW_DELSTRUCT) {
                DeleteStruct();
                UpdateStructMenu();
/*
#if defined(MOLFLOW)
                //worker.CalcTotalOutgassing();
#endif
*/
                return true;
            }
            else if (src->GetId() == MENU_VIEW_PREVSTRUCT) {
                if (interfGeom->viewStruct == -1) interfGeom->viewStruct = interfGeom->GetNbStructure() - 1;
                else
                    interfGeom->viewStruct = (int)Previous(interfGeom->viewStruct, interfGeom->GetNbStructure());
                interfGeom->UnselectAll();
                UpdateStructMenu();
                return true;
            }
            else if (src->GetId() == MENU_VIEW_NEXTSTRUCT) {
                interfGeom->viewStruct = (int) Next(interfGeom->viewStruct, interfGeom->GetNbStructure());
                interfGeom->UnselectAll();
                UpdateStructMenu();
                return true;
            }

                // Select selection
            else if (MENU_SELECTION_SELECTIONS + selections.size() > src->GetId() &&
                     src->GetId() >= MENU_SELECTION_SELECTIONS) { //Choose selection by number
                SelectSelection(src->GetId() - MENU_SELECTION_SELECTIONS);
                return true;
            } else if (src->GetId() == (MENU_SELECTION_SELECTIONS + selections.size())) { //Previous selection
                SelectSelection(Previous(idSelection, selections.size()));
                return true;
            } else if (src->GetId() == (MENU_SELECTION_SELECTIONS + selections.size() + 1)) { //Next selection
                SelectSelection(Next(idSelection, selections.size()));
                return true;
            }

                // Clear selection
            else if (src->GetId() >= MENU_SELECTION_CLEARSELECTIONS &&
                     src->GetId() < MENU_SELECTION_CLEARSELECTIONS + selections.size()) {
                char tmpname[256];
                sprintf(tmpname, "Clear %s?", selections[src->GetId() - MENU_SELECTION_CLEARSELECTIONS].name.c_str());
                if (GLMessageBox::Display(tmpname, "Confirmation", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) ==
                    GLDLG_OK) {
                    ClearSelection(src->GetId() - MENU_SELECTION_CLEARSELECTIONS);
                }
                return true;
            }

                // Memorize selection
            else if (src->GetId() >= MENU_SELECTION_MEMORIZESELECTIONS &&
                     src->GetId() < MENU_SELECTION_MEMORIZESELECTIONS + selections.size()) {
                OverWriteSelection(src->GetId() - MENU_SELECTION_MEMORIZESELECTIONS);
                return true;
            }

                // Select view
            else if (src->GetId() >= MENU_VIEW_VIEWS && src->GetId() < MENU_VIEW_VIEWS + views.size()) {
                SelectView(src->GetId() - MENU_VIEW_VIEWS);
                return true;
            }
                // Clear view
            else if (src->GetId() >= MENU_VIEW_CLEARVIEWS && src->GetId() < MENU_VIEW_CLEARVIEWS + views.size()) {
                char tmpname[256];
                sprintf(tmpname, "Clear %s?", views[src->GetId() - MENU_VIEW_CLEARVIEWS].name.c_str());
                if (GLMessageBox::Display(tmpname, "Confirmation", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONINFO) ==
                    GLDLG_OK) {
                    ClearView(src->GetId() - MENU_VIEW_CLEARVIEWS);
                }
                return true;
            }
                // Memorize view
            else if (src->GetId() >= MENU_VIEW_MEMORIZEVIEWS && src->GetId() < MENU_VIEW_MEMORIZEVIEWS + views.size()) {
                OverWriteView(src->GetId() - MENU_VIEW_MEMORIZEVIEWS);
                return true;
            }
            else if (src->GetId() == MENU_VIEW_CROSSSECTION) {
                if (!crossSectionWindow) crossSectionWindow = new CrossSection(interfGeom,&worker,curViewer);
                crossSectionWindow->SetVisible(true);
                return true;
            }
            break;

            //LIST --------------------------------------------------------------------
        case MSG_LIST:
            if (src == facetList && interfGeom->IsLoaded()) {
                auto selRows = facetList->GetSelectedRows(true);
                interfGeom->UnselectAll();
                for (auto &sel:selRows)
                    interfGeom->SelectFacet(sel);
                interfGeom->UpdateSelection();
                UpdateFacetParams(false);
                return true;
            }
            break;

            //GEOMVIEWER ------------------------------------------------------------------
        case MSG_GEOMVIEWER_MAXIMISE: {
            if (src == viewers[0]) {
                AnimateViewerChange(0);
            } else if (src == viewers[1]) {
                AnimateViewerChange(1);
            } else if (src == viewers[2]) {
                AnimateViewerChange(2);
            } else if (src == viewers[3]) {
                AnimateViewerChange(3);
            }
            Place3DViewer();

            bool neededTexture = needsTexture;

            bool neededMesh = needsMesh;
            CheckNeedsTexture();

            if (!needsTexture && neededTexture) { //We just disabled textures
                worker.GetGeometry()->ClearFacetTextures();
            } else if (needsTexture && !neededTexture) { //We just enabled textures
                worker.RebuildTextures();
            }

            if (!needsMesh && neededMesh) { //We just disabled mesh
                interfGeom->ClearFacetMeshLists();
            } else if (needsMesh && !neededMesh) { //We just enabled mesh
                interfGeom->BuildFacetMeshLists();
            }

            return true;
        }
        case MSG_GEOMVIEWER_SELECT: {
            SelectViewer(src->GetId());
            return true;
        }

            //BUTTON ------------------------------------------------------------------
        case MSG_BUTTON:
            if (src == resetSimu) {
                changedSinceSave = true;
                ResetSimulation();
                return true;
            } else if (src == forceFrameMoveButton) {
                updateRequested = true;
                //FrameMove();
                return true;
            } else if (src == facetCoordBtn) {
                if (!facetCoordinates) facetCoordinates = new FacetCoordinates();
                facetCoordinates->Display(&worker);
                return true;
            }
            break;

            //Panel open/close ---------------------------------------------------------
        case MSG_PANELR:
            PlaceComponents();
            return true;
        default:
            return false;
    }
    return false;
}

void Interface::BeforeExit()
{
    auto prg = GLProgress_GUI("Saving config file...", "Exiting...");
    prg.SetVisible(true);
    SaveConfig();
    if (appUpdater) {
        appUpdater->IncreaseSessionCount();
    }
    prg.SetMessage("Removing autosave file...");
    remove(autosaveFilename.c_str());
    auto cwd = std::filesystem::current_path();
    auto tempDir = cwd / "tmp";
    prg.SetMessage(fmt::format("Flushing temp directory\n{} ...\nIf this takes long, indexing or a cloud sync client is locking the file.", tempDir.string()));
    std::filesystem::remove_all(tempDir);
    prg.SetProgress(1.0);
}

void Interface::CheckNeedsTexture() {
    needsMesh = needsTexture = needsDirection = false;
    for (auto & view : viewers) {
        needsMesh = needsMesh || (view->IsVisible() && view->showMesh);
        needsTexture = needsTexture || (view->IsVisible() && view->showTexture);
        needsDirection = needsDirection || (view->IsVisible() && view->showDir);
    }
}

//SELECTIONS

void Interface::SelectView(int v) {
    viewers[curViewer]->SetCurrentView(views[v]);
}

void Interface::SelectSelection(size_t v) {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    interfGeom->SetSelection(selections[v].facetIds, viewers[0]->GetWindow()->IsShiftDown(),
                       viewers[0]->GetWindow()->IsCtrlDown());
    idSelection = v;
}

void Interface::ClearSelectionMenus() const {
    memorizeSelectionsMenu->Clear();
    memorizeSelectionsMenu->Add("Add new...", MENU_SELECTION_ADDNEW, SDLK_w, CTRL_MODIFIER);
    memorizeSelectionsMenu->Add(nullptr); // Separator
    clearSelectionsMenu->Clear();
    clearSelectionsMenu->Add("Clear All", MENU_SELECTION_CLEARALL);
    clearSelectionsMenu->Add(nullptr); // Separator
    selectionsMenu->Clear();
}

void Interface::RebuildSelectionMenus() {
    ClearSelectionMenus();
    size_t i;
    for (i = 0; i < selections.size(); i++) {
        if (i <= 8) {
            selectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_SELECTIONS + (int) i, SDLK_1 + (int) i,
                                ALT_MODIFIER);
        } else {
            selectionsMenu->Add(selections[i].name.c_str(),
                                MENU_SELECTION_SELECTIONS + (int) i); //no place for ALT+shortcut
        }
        clearSelectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_CLEARSELECTIONS + (int) i);
        memorizeSelectionsMenu->Add(selections[i].name.c_str(), MENU_SELECTION_MEMORIZESELECTIONS + (int) i);
    }
    selectionsMenu->Add(nullptr); //Separator
    selectionsMenu->Add("Select previous", MENU_SELECTION_SELECTIONS + (int) i, SDLK_F11, ALT_MODIFIER);
    selectionsMenu->Add("Select next", MENU_SELECTION_SELECTIONS + (int) i + 1, SDLK_F12, ALT_MODIFIER);
}

void Interface::AddSelection(const SelectionGroup& s) {
    selections.push_back(s);
    RebuildSelectionMenus();
}

void Interface::ClearSelection(size_t idClr) {
    selections.erase(selections.begin() + idClr);
    RebuildSelectionMenus();
}

void Interface::ClearAllSelections() {
    selections.clear();
    ClearSelectionMenus();
}

void Interface::OverWriteSelection(size_t idOvr) {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char *selectionName = GLInputBox::GetInput(selections[idOvr].name.c_str(), "Selection name",
                                               "Enter selection name");
    if (!selectionName) return;

    selections[idOvr].facetIds = interfGeom->GetSelectedFacets();
    selections[idOvr].name = selectionName;
    RebuildSelectionMenus();
}

void Interface::AddSelection(const std::string &selectionName) {
    if (selectionName.empty()) return;

    SelectionGroup newSelection;
    newSelection.facetIds = worker.GetGeometry()->GetSelectedFacets();
    newSelection.name = selectionName;
    selections.push_back(newSelection);
    RebuildSelectionMenus();
}

//VIEWS

void Interface::ClearViewMenus() const {
    memorizeViewsMenu->Clear();
    memorizeViewsMenu->Add("Add new...", MENU_VIEW_ADDNEW, SDLK_q, CTRL_MODIFIER);
    memorizeViewsMenu->Add(nullptr); // Separator
    clearViewsMenu->Clear();
    clearViewsMenu->Add("Clear All", MENU_VIEW_CLEARALL);
    clearViewsMenu->Add(nullptr); // Separator
    viewsMenu->Clear();
}

void Interface::RebuildViewMenus() {
    ClearViewMenus();
    std::vector<int> fKeys = {SDLK_F1, SDLK_F2, SDLK_F3, SDLK_F5, SDLK_F6, SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10,
                              SDLK_F11, SDLK_F12}; //Skip ALT+F4 shortcut :)
    for (int i = 0; i < views.size(); i++) {
        if (fKeys.empty()) {
            viewsMenu->Add(views[i].name.c_str(), MENU_VIEW_VIEWS + i);
        } else {
            viewsMenu->Add(views[i].name.c_str(), MENU_VIEW_VIEWS + i, fKeys[0], ALT_MODIFIER);
            fKeys.erase(fKeys.begin());
        }
        clearViewsMenu->Add(views[i].name.c_str(), MENU_VIEW_CLEARVIEWS + i);
        memorizeViewsMenu->Add(views[i].name.c_str(), MENU_VIEW_MEMORIZEVIEWS + i);
    }
}

void Interface::AddView(const CameraView& v) {

    if (views.size() >= MAX_VIEW) {
        views.erase(views.begin()); //clear first
    }
    views.push_back(v);
    RebuildViewMenus();
}

void Interface::ClearView(int idClr) {
    views.erase(views.begin()+idClr);
    RebuildViewMenus();
}

void Interface::ClearAllViews() {
    views.clear();
    ClearViewMenus();
}

void Interface::OverWriteView(int idOvr) {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char *viewName = GLInputBox::GetInput(views[idOvr].name.c_str(), "View name", "Enter view name");
    if (!viewName) return;

    views[idOvr] = viewers[curViewer]->GetCurrentView();
    views[idOvr].name = viewName;
    RebuildViewMenus();
}

void Interface::AddView() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char tmp[32];
    sprintf(tmp, "View #%zd", views.size() + 1);
    char *viewName = GLInputBox::GetInput(tmp, "View name", "Enter view name");
    if (!viewName) return;
    CameraView newView = viewers[curViewer]->GetCurrentView(); //Copy current view
    newView.name = viewName; //Apply user-defined name
    AddView(newView);
    RebuildViewMenus();
}

void Interface::RemoveRecent(const std::string& fileName) {

    if (fileName.empty()) return;
    bool found = false;

    auto pos=std::find(recentsList.begin(),recentsList.end(),fileName);
    if (pos!=recentsList.end()) {
        recentsList.erase(pos);
    }

    // Update menu
    UpdateRecentMenu();
    SaveConfig();
}

void Interface::AddRecent(const std::string &fileName) {

    if (fileName.empty()) return;

    auto pos=std::find(recentsList.begin(),recentsList.end(),fileName);
    if (pos!=recentsList.end()) {
        recentsList.erase(pos);
    }

    // Add new element
    recentsList.push_back(fileName);

    // Remove oldest elements exceedint the limit
    while (recentsList.size() >= MAX_RECENT) {
        recentsList.erase(recentsList.begin()); //Remove first=oldest
    }

    // Update menu
    UpdateRecentMenu();
    SaveConfig();
}

void Interface::UpdateRecentMenu() {
    // Update menu
    auto m = menu->GetSubMenu("File")->GetSubMenu("Load recent");
    m->Clear();
    for (int i = recentsList.size() - 1;i>=0;i--) { //Add in reverse order (last in vector is latest file)
        m->Add(AbbreviateString(recentsList[i], MAX_ITEM_LGTH - 1), MENU_FILE_LOADRECENT + i);
    }
}

void Interface::AddStruct() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    char tmp[32];
    sprintf(tmp, "Structure #%zd", interfGeom->GetNbStructure() + 1);
    char *structName = GLInputBox::GetInput(tmp, "Structure name", "Enter name of new structure");
    if (!structName) return;
    interfGeom->AddStruct(structName);
    // Send to sub process
    worker.MarkToReload();
}

void Interface::DeleteStruct() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (interfGeom->GetNbStructure() <= 1) {
        GLMessageBox::Display("At least one structure needs to remain.");
        return;
    }
    char *structNum = GLInputBox::GetInput("", "Structure number", "Number of structure to delete:");
    if (!structNum) return;
    int structNumInt;
    if (!sscanf(structNum, "%d", &structNumInt)) {
        GLMessageBox::Display("Invalid structure number. Can't parse");
        return;
    }
    if (structNumInt < 1 || structNumInt > interfGeom->GetNbStructure()) {
        GLMessageBox::Display("This structure doesn't exist.");
        return;
    }
    bool hasFacets = false;
    for (int i = 0; i < interfGeom->GetNbFacet() && !hasFacets; i++) {
        if (interfGeom->GetFacet(i)->sh.superIdx == (structNumInt - 1)) hasFacets = true;
    }
    if (hasFacets) {
        int rep = GLMessageBox::Display("This structure has facets. They will be deleted with the structure.",
                                        "Structure delete", GLDLG_OK | GLDLG_CANCEL, GLDLG_ICONWARNING);
        if (rep != GLDLG_OK) return;
    }
    if (!AskToReset()) return;
    interfGeom->DelStruct(structNumInt - 1);
    // Send to sub process
    worker.MarkToReload();
}

void Interface::DisplayCollapseDialog() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (!collapseSettings) collapseSettings = new CollapseSettings();
    collapseSettings->SetGeometry(interfGeom, &worker);
    collapseSettings->SetVisible(true);
}

void Interface::RenumberSelections(const std::vector<int> &newRefs) {
    for (int i = 0; i < selections.size(); i++) {
        for (int j = 0; i >= 0 && i < selections.size() && j >= 0 && j < selections[i].facetIds.size(); j++) {
            if (selections[i].facetIds[j] >= newRefs.size() ||
                newRefs[selections[i].facetIds[j]] == -1) { //remove from selection
                selections[i].facetIds.erase(selections[i].facetIds.begin() + j);
                j--; //Do again the element as now it's the next
                if (selections[i].facetIds.empty()) {
                    ClearSelection(i); //last facet removed from selection
                    i--;
                }

            } else { //renumber
                selections[i].facetIds[j] = newRefs[selections[i].facetIds[j]];
            }
        }
    }
}

void Interface::RenumberFormulas(std::vector<int> *newRefs) const {
    for (auto &f:appFormulas->formulas) {
        std::string expression = f.GetExpression();
        if (OffsetFormula(expression, 0, -1, newRefs)) { //modify in-place
            f.SetExpression(expression);
            f.Parse();
        }
    }
    if (formulaEditor && formulaEditor->IsVisible()) formulaEditor->Refresh();
}

void Interface::ClearFormulas() const {
    appFormulas->ClearFormulas();
    if (formulaEditor) formulaEditor->Refresh();
}

bool Interface::OffsetFormula(std::string& expression, int offset, int filter, std::vector<int> *newRefs) {
    //will increase or decrease facet numbers in a formula
    //only applies to facet numbers larger than "filter" parameter
    //If *newRefs is not nullptr, a vector is passed containing the new references
    bool changed = false;

    std::string newExpr = expression; //convert char* to string

    size_t pos = 0; //analyzed until this position
    while (pos < newExpr.size()) { //while not end of expression

        std::vector<size_t> location; //for each prefix, we store where it was found

        for (auto & formulaPrefix : formulaPrefixes) { //try all expressions
            location.push_back(newExpr.find(formulaPrefix, pos));
        }
        size_t minPos = std::string::npos;
        size_t maxLength = 0;
        for (size_t j = 0; j < formulaPrefixes.size(); j++)  //try all expressions, find first prefix location
            if (location[j] < minPos) minPos = location[j];
        for (size_t j = 0; j < formulaPrefixes.size(); j++)  //try all expressions, find longest prefix at location
            if (location[j] == minPos && formulaPrefixes[j].size() > maxLength) maxLength = formulaPrefixes[j].size();
        int digitsLength = 0;
        if (minPos != std::string::npos) { //found expression, let's find tailing facet number digits
            while ((minPos + maxLength + digitsLength) < newExpr.length() &&
                   newExpr[minPos + maxLength + digitsLength] >= '0' &&
                   newExpr[minPos + maxLength + digitsLength] <= '9')
                digitsLength++;
            if (digitsLength > 0) { //there was a digit after the prefix
                int facetNumber;
                if (sscanf(newExpr.substr(minPos + maxLength, digitsLength).c_str(), "%d", &facetNumber)) {
                    if (newRefs == nullptr) { //Offset mode
                        if ((facetNumber - 1) > filter) {
                            char tmp[10];
                            sprintf(tmp, "%d", facetNumber + offset);
                            newExpr.replace(minPos + maxLength, digitsLength, tmp);
                            changed = true;
                        } else if ((facetNumber - 1) == filter) {
                            newExpr.replace(minPos + maxLength, digitsLength, "0");
                            changed = true;
                        }
                    } else { //newRefs mode
                        if ((facetNumber - 1) >= (*newRefs).size() ||
                            (*newRefs)[facetNumber - 1] == -1) { //Facet doesn't exist anymore
                            newExpr.replace(minPos + maxLength, digitsLength, "0");
                            changed = true;
                        } else { //Update facet number
                            char tmp[12];
                            sprintf(tmp, "%d", (*newRefs)[facetNumber - 1] + 1);
                            newExpr.replace(minPos + maxLength, digitsLength, tmp);
                            changed = true;
                        }
                    }
                }
            }
        }
        if (minPos != std::string::npos) pos = minPos + maxLength + digitsLength;
        else pos = minPos;
    }
    expression = newExpr;
    return changed;
}

int Interface::Resize(size_t width, size_t height, bool forceWindowed) {
    int r = GLApplication::Resize(width, height, forceWindowed);
    PlaceComponents();
    worker.RebuildTextures();
    return r;
}

void Interface::UpdateFacetlistSelected() {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    auto selectedFacets = interfGeom->GetSelectedFacets();
    //facetList->SetSelectedRows(selection,nbSelected,true);
    if (selectedFacets.size() > 1000) {
        facetList->ReOrder();
        facetList->SetSelectedRows(selectedFacets, false);
    } else {
        facetList->SetSelectedRows(selectedFacets, true);
    }
}

void Interface::DropEvent(char *dropped_file) {
    // Shows directory of dropped file
    int ret = GLMessageBox::Display(fmt::format("Do you want to load this file\n    {}?", dropped_file).c_str(), "Load file?",
            GLDLG_OK|GLDLG_CANCEL,GLDLG_ICONWARNING);
    /**/
    if(ret == GLDLG_OK) {
        LoadFile(dropped_file);
    }
}

bool Interface::AskToSave() {
    if (!changedSinceSave) return true;
    int ret = GLSaveDialog::Display("Save current geometry first?", "File not saved",
                                    GLDLG_SAVE | GLDLG_DISCARD | GLDLG_CANCEL_S, GLDLG_ICONINFO);
    if (ret == GLDLG_SAVE) {
        std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");
        if (!fn.empty()) {
            auto prg = GLProgress_GUI("Saving file...", "Please wait");
            prg.SetVisible(true);
            //GLWindowManager::Repaint();
            try {
                worker.SaveGeometry(fn, prg);
                changedSinceSave = false;
                UpdateTitle();
                AddRecent(fn);
            }
            catch (const std::exception &e) {
                char errMsg[512];
                sprintf(errMsg, "%s\nFile:%s", e.what(), fn.c_str());
                GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
                RemoveRecent(fn.c_str());
            }
            return true;
        } else return false;
    } else if (ret == GLDLG_DISCARD) return true;
    return false;
}

void Interface::CreateOfTwoFacets(Clipper2Lib::ClipType type, int reverseOrder) {
    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (interfGeom->IsLoaded()) {
        try {
            if (AskToReset()) {
                //interfGeom->CreateDifference();
                interfGeom->ClipSelectedPolygons(type, reverseOrder);
            }
        }
        catch (const std::exception &e) {
            GLMessageBox::Display(e.what(), "Error creating polygon", GLDLG_OK, GLDLG_ICONERROR);
        }
        //UpdateModelParams();
        worker.MarkToReload();
    } else GLMessageBox::Display("No geometry loaded.", "No geometry", GLDLG_OK, GLDLG_ICONERROR);
}

void Interface::SaveFileAs() {

    std::string fn = NFD_SaveFile_Cpp(fileSaveFilters, "");

    auto prg = GLProgress_GUI("Saving file...", "Please wait");
    prg.SetVisible(true);
    //GLWindowManager::Repaint();
    if (!fn.empty()) {

        try {

            worker.SaveGeometry(fn, prg);
            ResetAutoSaveTimer();
            changedSinceSave = false;
            UpdateTitle();
            AddRecent(worker.fullFileName);
        }
        catch (const std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nFile:%s", e.what(), fn.c_str());
            GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
            RemoveRecent(fn.c_str());
        }

    }
}

void Interface::ExportTextures(int grouping, int mode) {

    InterfaceGeometry *interfGeom = worker.GetGeometry();
    if (interfGeom->GetNbSelectedFacets() == 0) {
        GLMessageBox::Display("Empty selection", "Error", GLDLG_OK, GLDLG_ICONERROR);
        return;
    }

    std::string fn = NFD_SaveFile_Cpp(fileTexFilters, "");
    if (!fn.empty()) {

        try {
            worker.ExportTextures(fn.c_str(), grouping, mode, true, true);
        }
        catch (const std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nFile:%s", e.what(), fn.c_str());
            GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);
        }

    }

}

void Interface::DoEvents(bool forced) {
    static int lastChkEvent = 0;
    static int lastRepaint = 0;
    int time = SDL_GetTicks();
    if (forced || (time - lastChkEvent > 200)) { //Don't check for inputs more than 5 times a second
        SDL_Event sdlEvent;
        SDL_PollEvent(&sdlEvent);
        UpdateEventCount(&sdlEvent);
        /*if (GLWindowManager::ManageEvent(&sdlEvent)) {
        // Relay to GLApp EventProc
        mApp->EventProc(&sdlEvent);
        }*/
        GLWindowManager::ManageEvent(&sdlEvent);
        lastChkEvent = time;
    }
    if (forced || (time - lastRepaint > 500)) { //Don't redraw more than every 500 msec
        GLWindowManager::Repaint();
        GLToolkit::CheckGLErrors("GLApplication::Paint()");
        lastRepaint = time;
    }

}

bool Interface::AskToReset(Worker *work) {
    if (work == nullptr) work = &worker;
    if (work->globalStatCache.globalHits.nbMCHit > 0 || work->IsRunning()) { //If running, maybe scene auto-update is disabled, so nbMCHit stays 0.
        int rep = GLMessageBox::Display("This will reset simulation data.", "Geometry change", GLDLG_OK | GLDLG_CANCEL,
                                        GLDLG_ICONWARNING);
        if (rep == GLDLG_OK) {
            ResetSimulation(false);
            return true;
        } else return false;
    } else return true;
}

int Interface::FrameMove() {
    char tmp[256];
    InterfaceGeometry *interfGeom = worker.GetGeometry();

    bool runningState = worker.IsRunning();

    //Autosave routines
    bool timeForAutoSave = false;
    if (interfGeom->IsLoaded()) {
        if (autoSaveSimuOnly) {
            if (runningState) {
                if (((worker.simuTimer.Elapsed()) - lastSaveTimeSimu) >=
                    (float) autoSaveFrequency * 60.0f) {
                    timeForAutoSave = true;
                }
            }
        } else {
            if ((m_fTime - lastSaveTime) >= (float) autoSaveFrequency * 60.0f) {
                timeForAutoSave = true;
            }
        }
    }

    auto& hitCache = worker.globalStatCache.globalHits;

    bool oneSecSinceLastUpdate = m_fTime - lastUpdate >= 1.0f;
    bool justStopped = !runningState && worker.simuTimer.isActive;
    if ((runningState && oneSecSinceLastUpdate) || justStopped) {
        
        
        sprintf(tmp, "Running: %s", Util::formatTime(worker.simuTimer.Elapsed()));
        sTime->SetText(tmp);
        wereEvents = true; //Will repaint

        UpdateStats(); //Update m_fTime
        lastUpdate = m_fTime;

        if (updateRequested || autoFrameMove  || justStopped) {

            forceFrameMoveButton->SetEnabled(false);
            forceFrameMoveButton->SetText("Updating...");
            //forceFrameMoveButton->Paint();
            //GLWindowManager::Repaint();

            updateRequested = false;

            // Update hits
            try {
                worker.Update(m_fTime);
            }
            catch (const std::exception &e) {
                GLMessageBox::Display(e.what(), "Error (Stop)", GLDLG_OK, GLDLG_ICONERROR);
            }
            // Simulation monitoring
            if (appFormulas->recordConvergence || (formulaEditor && formulaEditor->IsVisible()) || (convergencePlotter && convergencePlotter->IsVisible())) appFormulas->EvaluateFormulas(hitCache.nbDesorbed);
            UpdatePlotters();

            // Formulas
            //if (autoUpdateFormulas) UpdateFormula();
            if (autoUpdateFormulas && formulaEditor && formulaEditor->IsVisible()) {
                formulaEditor->UpdateValues();
            }
            if (particleLogger && particleLogger->IsVisible()) particleLogger->UpdateStatus();
            //lastUpdate = GetTick(); //changed from m_fTime: include update duration

            // Update timing measurements
            if (hitCache.nbMCHit != lastNbHit ||
                hitCache.nbDesorbed != lastNbDes) {
                auto dTime = (double) (m_fTime - lastMeasTime);
                if(dTime > 1e-6) {
                    hps.push(hitCache.nbMCHit - lastNbHit, m_fTime);
                    dps.push(hitCache.nbDesorbed - lastNbDes, m_fTime);
                    //hps = (double) (hitCache.nbMCHit - lastNbHit) / dTime;
                    //dps = (double) (hitCache.nbDesorbed - lastNbDes) / dTime;
                }

                lastNbHit = hitCache.nbMCHit;
                lastNbDes = hitCache.nbDesorbed;
                lastMeasTime = m_fTime;
            }
        }
        

        forceFrameMoveButton->SetEnabled(!autoFrameMove);
        forceFrameMoveButton->SetText("Update");
    } else {
        /* //Commenting out, caused constant redraw when simulation paused
        if(!runningState && worker.simuTimer.Elapsed() > 0.0) { //Paused
            double _hps = (double) (hitCache.nbMCHit - nbHitStart) / worker.simuTimer.Elapsed();
            double _dps = (double) (hitCache.nbDesorbed - nbDesStart) / worker.simuTimer.Elapsed();
            if (hps.last() != _hps || dps.last() != _dps) {
                wereEvents = true;
            }
        } else { //Stopped?
            if (hps.last() != 0.0 || dps.last() != 0.0) {
                wereEvents = true;
            }
        }
        */

        if(runningState)
            sprintf(tmp, "Running: %s", Util::formatTime(worker.simuTimer.Elapsed()));
        else
            sprintf(tmp, "Stopped: %s", Util::formatTime(worker.simuTimer.Elapsed()));
        sTime->SetText(tmp);
    }


    // Facet parameters and hits
    if (viewers[0]->SelectionChanged() ||
        viewers[1]->SelectionChanged() ||
        viewers[2]->SelectionChanged() ||
        viewers[3]->SelectionChanged()) {
        UpdateFacetParams(true);
    }
    UpdateFacetHits(false);
    //Autosave
    if (timeForAutoSave) AutoSave();

    //Check if app updater has found updates
    if (appUpdater) {
        if(appUpdater->IsUpdateAvailable()) {
            if (!updateLogWindow) {
                updateLogWindow = new UpdateLogWindow(this);
            }
            if (!updateFoundDialog) {
                updateFoundDialog = new UpdateFoundDialog(appName, appVersionName, appUpdater, updateLogWindow);
                updateFoundDialog->SetVisible(true);
                wereEvents = true;
            }
        }
        appUpdater->NotifyServerWarning();
    }

    if (worker.globalStatCache.nbLeakTotal) {
        sprintf(tmp, "%g (%.4f%%)", (double) worker.globalStatCache.nbLeakTotal,
                (double) (worker.globalStatCache.nbLeakTotal) * 100.0 /
                (double) hitCache.nbDesorbed);
        leakNumber->SetText(tmp);
    } else {
        leakNumber->SetText("None");
    }
    resetSimu->SetEnabled(!runningState && hitCache.nbDesorbed > 0);

    if (runningState) {
        startSimu->SetText("Pause");
        //startSimu->SetFontColor(255, 204, 0);
    } else if (hitCache.nbMCHit > 0) {
        startSimu->SetText("Resume");
        //startSimu->SetFontColor(0, 140, 0);
    } else {
        startSimu->SetText("Begin");
        //startSimu->SetFontColor(0, 140, 0);
    }

    if(convergencePlotter && formulaEditor && appFormulas->convergenceDataChanged) {
        convergencePlotter->Refresh();
        appFormulas->convergenceDataChanged = false;
    }

    /*
    // Sleep a bit to avoid unwanted CPU load
    if (viewers[0]->IsDragging() ||
        viewers[1]->IsDragging() ||
        viewers[2]->IsDragging() ||
        viewers[3]->IsDragging() || !worker.running)
    {
        SDL_Delay(22); //was 22
    }
    else
    {
        SDL_Delay(60); //was 60
    }
    */


    /*if(imWin) {
        imWin->renderSingle();
    }*/

    double delayTime = 0.03 - (wereEvents ? fPaintTime : 0.0) - fMoveTime;
    if (delayTime > 0.0) { //static casting a double<-1 to uint is an underflow on Windows!
        auto delay_u = static_cast<uint32_t>(1000.0 * delayTime);
        if (delay_u < 50) {
            SDL_Delay(delay_u); //Limits framerate at about 60fps
        }
    }

    return GL_OK;
}

void Interface::ResetAutoSaveTimer() {
    UpdateStats(); //updates m_fTime
    if (autoSaveSimuOnly) lastSaveTimeSimu = worker.simuTimer.Elapsed();
    else lastSaveTime = m_fTime;
}

bool Interface::AutoSave(bool crashSave) {
    if (!changedSinceSave) return true;
    auto prg = GLProgress_GUI("Peforming autosave...", "Please wait");
    prg.SetVisible(true);
    //GLWindowManager::Repaint();

    std::string shortFn(worker.GetCurrentShortFileName());
    std::string newAutosaveFilename = appName + "_Autosave";
    if (!shortFn.empty()) newAutosaveFilename += "(" + shortFn + ")";
#if defined(MOLFLOW)
    newAutosaveFilename += ".zip";
#endif
#if defined(SYNRAD)
    newAutosaveFilename += ".syn7z";
#endif
    char fn[1024];
    strcpy(fn, newAutosaveFilename.c_str());
    try {
        worker.SaveGeometry(fn, prg, false, false, true, crashSave);
        //Success:
        if (!autosaveFilename.empty() && autosaveFilename != newAutosaveFilename) remove(autosaveFilename.c_str());
        autosaveFilename = newAutosaveFilename;
        ResetAutoSaveTimer(); //deduct saving time from interval
    }
    catch (const std::exception &e) {
        GLMessageBox::Display(std::string(e.what()) + "\n" + fn, "Autosave error", {"OK"}, GLDLG_ICONERROR);
        ResetAutoSaveTimer();
        return false;
    }
    //lastSaveTime=(worker.simuTime+(m_fTime-worker.startTime));
    wereEvents = true;
    return true;
}

void Interface::CheckForRecovery() {
    // Check for autosave files in current dir.
    auto curPath = std::filesystem::current_path(); //string (POSIX) or wstring (Windows)
    for (const auto &p : std::filesystem::directory_iterator(curPath)) {

        std::string path_str = p.path().u8string();
        std::string fileName_str = p.path().filename().u8string();

        if (beginsWith(fileName_str, appName + "_Autosave")) {
            std::ostringstream msg;
            msg << "Autosave file found:\n" << fileName_str << "\n";
            int rep = RecoveryDialog::Display(msg.str().c_str(), "Autosave recovery", GLDLG_LOAD | GLDLG_SKIP,
                                              GLDLG_DELETE);
            if (rep == GLDLG_LOAD) {
                LoadFile(path_str);
                RemoveRecent(path_str.c_str());
            } else if (rep == GLDLG_CANCEL_R) return;
            else if (rep == GLDLG_SKIP) continue;
            else if (rep == GLDLG_DELETE) remove(p.path());
        }
    }
}

void Interface::SetDefaultViews() {
    viewers[0]->SetProjection(ProjectionMode::Orthographic);
    viewers[0]->ToFrontView();
    viewers[1]->SetProjection(ProjectionMode::Orthographic);
    viewers[1]->ToTopView();
    viewers[2]->SetProjection(ProjectionMode::Orthographic);
    viewers[2]->ToSideView();
    viewers[3]->SetProjection(ProjectionMode::Perspective);
    viewers[3]->ToFrontView();
    SelectViewer(0);
}

void Interface::RefreshPlotterCombos() {
    //Removes non-present views, rebuilds combobox and refreshes plotted data
    if (histogramPlotter) histogramPlotter->Refresh();
    if (convergencePlotter) convergencePlotter->Refresh();
}