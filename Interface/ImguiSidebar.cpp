

#include "ImguiSidebar.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "../../src/MolflowGeometry.h"
#else
#include "../../src/SynRad.h"
#include "../../src/SynradGeometry.h"
#endif

#include "Facet_shared.h"
#include "../../src/Interface/Viewer3DSettings.h"

#include "imgui.h"
#include <imgui_internal.h>
#include <Helper/FormatHelper.h>
#include <Helper/StringHelper.h>
#include "Helper/MathTools.h"
#include "ImguiWindow.h"


// Dummy data structure that we use for the Table demo.
// (pre-C++11 doesn't allow us to instantiate ImVector<MyItem> template if this structure if defined inside the demo function)
namespace {
// We are passing our own identifier to TableSetupColumn() to facilitate identifying columns in the sorting code.
// This identifier will be passed down into ImGuiTableSortSpec::ColumnUserID.
// But it is possible to omit the user id parameter of TableSetupColumn() and just use the column index instead! (ImGuiTableSortSpec::ColumnIndex)
// If you don't use sorting, you will generally never care about giving column an ID!
    enum FacetDataColumnID {
        FacetDataColumnID_ID,
        FacetDataColumnID_Hits,
        FacetDataColumnID_Des,
        FacetDataColumnID_Abs
    };

    struct FacetData {
        int ID;
        size_t hits;
        size_t des;
        double abs;

        // We have a problem which is affecting _only this demo_ and should not affect your code:
        // As we don't rely on std:: or other third-party library to compile dear imgui, we only have reliable access to qsort(),
        // however qsort doesn't allow passing user data to comparing function.
        // As a workaround, we are storing the sort specs in a static/global for the comparing function to access.
        // In your own use case you would probably pass the sort specs to your sorting/comparing functions directly and not use a global.
        // We could technically call ImGui::TableGetSortSpecs() in CompareWithSortSpecs(), but considering that this function is called
        // very often by the sorting algorithm it would be a little wasteful.
        static const ImGuiTableSortSpecs *s_current_sort_specs;

        // Compare function to be used by qsort()
        static int IMGUI_CDECL CompareWithSortSpecs(const void *lhs, const void *rhs) {
            const FacetData *a = (const FacetData *) lhs;
            const FacetData *b = (const FacetData *) rhs;
            for (int n = 0; n < s_current_sort_specs->SpecsCount; n++) {
                // Here we identify columns using the ColumnUserID value that we ourselves passed to TableSetupColumn()
                // We could also choose to identify columns based on their index (sort_spec->ColumnIndex), which is simpler!
                const ImGuiTableColumnSortSpecs *sort_spec = &s_current_sort_specs->Specs[n];
                int delta = 0;
                switch (sort_spec->ColumnUserID) {
                    case FacetDataColumnID_ID:
                        delta = (a->ID - b->ID);
                        break;
                    case FacetDataColumnID_Hits:
                        delta = (a->hits > b->hits) ? 1 : (a->hits == b->hits) ? 0 : -1;
                        break;
                    case FacetDataColumnID_Des:
                        delta = (a->des > b->des) ? 1 : (a->des == b->des) ? 0 : -1;
                        break;
                    case FacetDataColumnID_Abs:
                        delta = (a->abs > b->abs) ? 1 : (a->abs == b->abs) ? 0 : -1;
                        break;
                    default:
                        IM_ASSERT(0);
                        break;
                }
                if (delta > 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? +1 : -1;
                if (delta < 0)
                    return (sort_spec->SortDirection == ImGuiSortDirection_Ascending) ? -1 : +1;
            }

            // qsort() is instable so always return a way to differenciate items.
            // Your own compare function may want to avoid fallback on implicit sort specs e.g. a Name compare if it wasn't already part of the sort specs.
            return (a->ID - b->ID);
        }
    };

    const ImGuiTableSortSpecs *FacetData::s_current_sort_specs = nullptr;
}


#if defined(MOLFLOW)
double ImGuiSidebar::PumpingSpeedFromSticking(double sticking, double area, double temperature, MolFlow* mApp) {
    return 1.0 * sticking * area / 10.0 / 4.0 * sqrt(8.0 * 8.31 * temperature / PI / (mApp->worker.model->sp.gasMass * 0.001));
}

double ImGuiSidebar::StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature, MolFlow* mApp) {
    return std::abs(pumpingSpeed / (area / 10.0) * 4.0 * sqrt(1.0 / 8.0 / 8.31 / (temperature) * PI * (mApp->worker.model->sp.gasMass * 0.001)));
}

// Sidebar containing 3d viewer settings, facet settings and simulation data
void ImGuiSidebar::ShowAppSidebar(bool *p_open, MolFlow *mApp, InterfaceGeometry *interfGeom) {
#else
void ImGuiSidebar::ShowAppSidebar(bool *p_open, SynRad *mApp, InterfaceGeometry *interfGeom) {
#endif
    const float PAD = 10.0f;
    static int corner = 0;
    static float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    static float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

    ImGuiIO &io = ImGui::GetIO();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    static bool use_work_area = true; // experiment with work area vs normal area

    static float width = TEXT_BASE_WIDTH * 30;

    ImGui::SetNextWindowPos(use_work_area ?
                            ImVec2(viewport->Size.x - width, viewport->WorkPos.y)
                                          : viewport->Pos);
    // Set size initially (to be able for resize) to take the full height on the right side
    ImGui::SetNextWindowSize(
            use_work_area ? ImVec2(width, viewport->WorkSize.y) : ImVec2(width, viewport->Size.y));

    static ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | 
            ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar;
    /*ImGuiWindowFlags_NoDecoration | *//*ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove *//*| ImGuiWindowFlags_NoResize *//*|
            ImGuiWindowFlags_NoSavedSettings;*/

    if (!use_work_area) // example from ShowExampleAppSimpleOverlay() [demo] for corner positioning
    {
        int corner = 1; // top right
        const float PAD = 0.0f;//const float PAD = 10.0f;
        const ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImVec2 work_pos = viewport->WorkPos; // Use work area to avoid menu-bar/task-bar, if any!
        ImVec2 work_size = viewport->WorkSize;
        ImVec2 window_pos, window_pos_pivot;
        window_pos.x = (corner & 1) ? (work_pos.x + work_size.x - PAD) : (work_pos.x + PAD);
        window_pos.y = (corner & 2) ? (work_pos.y + work_size.y - PAD) : (work_pos.y + PAD);
        window_pos_pivot.x = (corner & 1) ? 1.0f : 0.0f;
        window_pos_pivot.y = (corner & 2) ? 1.0f : 0.0f;
        ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        flags |= ImGuiWindowFlags_NoMove;
    }
    //ImGui::SetNextWindowSize(ImVec2(viewport->WorkSize.x - viewport->WorkSize.x * 0.8f, viewport->WorkSize.y));
    static bool displayedWarning = false;
#ifndef DEBUG
    if (!displayedWarning) {
        ImIOWrappers::InfoPopup("WARNING", "Sidebar is not properly implemented yet. Most features do not work!");
        displayedWarning = true;
    }
#endif
    if (ImGui::Begin("[BETA] Molflow Sidebar", p_open, flags)) {
#if defined(DEBUG)
        if (p_open && ImGui::Button("Close this window"))
            *p_open = false;
        if (ImGui::CollapsingHeader("[DEMO] Window flags")) {

            ImGui::Checkbox("Use work area instead of main area", &use_work_area);
            ImGui::SameLine();
            ImGui::HelpMarker(
                    "Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.");

            ImGui::CheckboxFlags("ImGuiWindowFlags_NoBackground", &flags, ImGuiWindowFlags_NoBackground);
            ImGui::CheckboxFlags("ImGuiWindowFlags_NoDecoration", &flags, ImGuiWindowFlags_NoDecoration);
            ImGui::Indent();
            ImGui::CheckboxFlags("ImGuiWindowFlags_NoTitleBar", &flags, ImGuiWindowFlags_NoTitleBar);
            ImGui::CheckboxFlags("ImGuiWindowFlags_NoCollapse", &flags, ImGuiWindowFlags_NoCollapse);
            ImGui::CheckboxFlags("ImGuiWindowFlags_NoScrollbar", &flags, ImGuiWindowFlags_NoScrollbar);
            ImGui::Unindent();

        }
#endif
        if (ImGui::CollapsingHeader("3D Viewer settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto curViewer = mApp->curViewer;
            auto viewer = mApp->viewers[curViewer];
            if (ImGui::BeginTable("table_3dviewer", 3, ImGuiTableFlags_None)) {
                /*ImGui::TableSetupColumn("col1");
                ImGui::TableSetupColumn("col2");
                ImGui::TableSetupColumn("col3");
                ImGui::TableHeadersRow();*/
                ImGui::TableNextRow();
                {
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Axes", &viewer->showRule);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Normals", &viewer->showNormal);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox(fmt::format("{}", u8"u\u20d7,v\u20d7").c_str(), &viewer->showUV);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Lines", &viewer->showLine);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Leaks", &viewer->showLeak);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Hits", &viewer->showHit);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Volume", &viewer->showVolume);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Texture", &viewer->showTexture);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("FacetIDs", &viewer->showFacetId);
                    ImGui::TableNextRow();
                    ImGui::TableNextColumn();
                    if(ImGui::Button("<< View")){
                        LockWrapper myLock(mApp->imguiRenderLock);
                        if (!mApp->viewer3DSettings)
                            mApp->viewer3DSettings = new Viewer3DSettings();
                        mApp->viewer3DSettings->SetVisible(!mApp->viewer3DSettings->IsVisible());
                        mApp->viewer3DSettings->Reposition();
                        auto curViewer = mApp->curViewer;
                        auto viewer = mApp->viewers[curViewer];
                        mApp->viewer3DSettings->Refresh(mApp->worker.GetGeometry(), viewer);
                    }
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Indices", &viewer->showIndex);
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("VertexIDs", &viewer->showVertexId);
                }

                ImGui::EndTable();

            }
        }

        std::string title;
        // remember last state to see if values need updating
        static int lastNbFacets = -1;
        static int lastFacetId = -1;
        size_t nbSelectedFacets = interfGeom->GetNbSelectedFacets();
        if(nbSelectedFacets > 1) {
            title = fmt::format("Selected Facet ({} selected)", nbSelectedFacets);
        }
        else if(nbSelectedFacets == 1){
            title = fmt::format("Selected Facet (#{})", interfGeom->GetSelectedFacets().front());
        }
        else {
            title = fmt::format("Selected Facet (none)");
        }
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            size_t selected_facet_id = nbSelectedFacets ? interfGeom->GetSelectedFacets().front() : 0;
            auto sel = nbSelectedFacets ? interfGeom->GetFacet(selected_facet_id) : nullptr;

            bool updateFacetInterfaceValues = (lastNbFacets != nbSelectedFacets || lastFacetId != selected_facet_id); // check if state changed
            lastNbFacets = static_cast<int>(nbSelectedFacets);
            lastFacetId = static_cast<int>(selected_facet_id);
            if (sel == nullptr) ImGui::BeginDisabled();
#if defined(MOLFLOW)
            if (ImGui::TreeNodeEx("Particles in", ImGuiTreeNodeFlags_DefaultOpen)) {
                // declare variables
                enum { use_og = 0, use_og_area = 1 };
                static int des_idx = 0;
                static std::string exponentInput = "";
                static std::string outgassingInput = "1.0";
                static std::string outgassingAreaInput = "1.0";
                static bool modeOfOg = use_og;
                static double og = 1.0;
                static double og_area = 1.0;
                /***********MISSING MIXED AND TIME-DEPENDENT STATES************/
                if (updateFacetInterfaceValues && sel) { // if selection changed, recalculate all values
                    des_idx = sel->sh.desorbType;
                    // TODO use time-depended sh.stickingParam if defined
                    og = sel->sh.outgassing * 10; // from bar to mbar
                    og_area = og / sel->sh.area;
                    outgassingInput = std::to_string(og);
                    outgassingAreaInput = std::to_string(og_area);
                }

                ImGui::Text("Desorption"); ImGui::SameLine();
                ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 10);
                // TODO handle mixed selection
                if (ImGui::Combo("##Desorption", &des_idx, u8"None\0Uniform\0Cosine\0Cosine\u207f\0Recorded\0")) {
                    sel->sh.desorbType = des_idx;
                    switch (des_idx) {
                        case 0:
                            fmt::print("none");
                            break;
                        case 1:
                            fmt::print("uni");
                            break;
                        case 2:
                            fmt::print("cos");
                            break;
                        case 3:
                            fmt::print(u8"cos \u207f");
                            break;
                        case 4:
                            fmt::print("rec");
                            break;
                    }
                }
                if (des_idx == 3) {
                    ImGui::SameLine();
                    ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 4);
                    ImGui::InputText("##exponent", &exponentInput);
                }

                if (ImGui::RadioButton(u8"Outgassing [mbar\u00b7l/s]", modeOfOg == use_og)) {
                    modeOfOg = use_og;
                }
                ImGui::SameLine();
                if (ImGui::InputText("##in", &outgassingInput)) {
                    modeOfOg = use_og;
                    // if value changed
                    if (Util::getNumber(&og, outgassingInput)) { // and conversion was a success
                        //update the other value
                        og_area = og / sel->sh.area;
                        outgassingAreaInput = std::to_string(og_area);
                        sel->sh.outgassing = og / 10;
                    }
                    else {
                        // TODO apply user string as time-depended parameter name
                    }
                }
                if (ImGui::RadioButton(u8"Outg/area [mbar\u00b7l/s/cm\u00b2]", modeOfOg == use_og_area)) {
                    modeOfOg = use_og_area;
                }
                ImGui::SameLine();
                if (ImGui::InputText("##ina", &outgassingAreaInput)) {
                    modeOfOg = use_og_area;
                    if (Util::getNumber(&og_area, outgassingAreaInput)) {
                        og = og_area * sel->sh.area;
                        outgassingInput = std::to_string(og);
                        sel->sh.outgassing = og / 10;
                    }
                }
                ImGui::TreePop();
            }
            bool updateFacetProperties = false;
            if (ImGui::TreeNodeEx("Particles out", ImGuiTreeNodeFlags_DefaultOpen)) {
                    static std::string sfInput = "1.0";
                    static std::string psInput = "1.0";
                    static double sf = 1.0;
                    static double ps = 1.0;

                if (updateFacetInterfaceValues && sel) { // if selection changed, recalculate all values
                    sf = sel->sh.sticking; 
                    sfInput = std::to_string(sf);
                    ps = PumpingSpeedFromSticking(sf,sel->sh.area,sel->sh.temperature, mApp);
                    psInput = std::to_string(ps);
                }
                ImGui::Text("Sticking Factor"); ImGui::SameLine();
                ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 10);
                if (ImGui::InputText("##StickingFactor", &sfInput)) {
                    if (Util::getNumber(&sf, sfInput)) {
                        // could work incorrectly correctly if area and temperature are changed
                        ps = PumpingSpeedFromSticking(sf, sel->sh.area, sel->sh.temperature, mApp);
                        psInput = std::to_string(ps);
                        updateFacetProperties = true;
                    }
                }
                ImGui::Text("Pumping speed [l/s]"); ImGui::SameLine();
                ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 10);
                if (ImGui::InputText("##PumpingSpeed", &psInput)) {
                    if (Util::getNumber(&ps, psInput)) {
                        // could work incorrectly correctly if area and temperature are changed
                        sf = StickingFromPumpingSpeed(ps, sel->sh.area, sel->sh.temperature, mApp);
                        sfInput = std::to_string(sf);
                        updateFacetProperties = true;
                    }
                }
                if (sf < 0 || sf > 1) { // if sf is out of range display warning
                    ImGui::TextColored(ImVec4(1,0,0,1), u8"Sticking factor \u2209[0,1]");
                }
                else if (sel && updateFacetProperties) { // and only assign sf to facet if it is in range
                    sel->sh.sticking = sf;
                }
                ImGui::TreePop();
            }
#endif
            {
                static int sides_idx = 0;
                if(updateFacetInterfaceValues && sel) sides_idx = sel->sh.is2sided;
                if (ImGui::Combo("Sides", &sides_idx, "1 Sided\0 2 Sided\0")) {
                    sel->sh.is2sided = sides_idx;
                    switch (sides_idx) {
                        case 0:
                            fmt::print("1s");
                            break;
                        case 1:
                            fmt::print("2s");
                            break;
                    }
                }

                static double opacity = 1.0;
                static std::string opacityInput = "1.0";
                updateFacetProperties = false;
                if (updateFacetInterfaceValues && sel) { // update if selection changed
                    opacity = sel->sh.opacity;
                    opacityInput = std::to_string(opacity);
                }
                ImGui::Text("Opacity"); ImGui::SameLine();
                ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 10);
                if (ImGui::InputText("##Opacity", &opacityInput)) { // if text changed
                    if (Util::getNumber(&opacity, opacityInput)) { // try to convert it to a number
                        updateFacetProperties = true; // if success set update flag
                    }
                }
                if (opacity < 0 || opacity > 1) { // if opacity is out of range display the warining
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"Opacity \u2209[0,1]");
                }
                else if (updateFacetProperties) { // set new facet opacity
                    sel->sh.opacity = opacity;
                }

#if defined(MOLFLOW)
                static double temp = 1.0;
                static std::string temperatureInput = "1.0";
                updateFacetProperties = false;
                if (updateFacetInterfaceValues && sel) {
                    temp = sel->sh.temperature;
                    temperatureInput = std::to_string(temp);
                }
                ImGui::Text(u8"Temperature [\u212a]"); ImGui::SameLine();
                ImGui::SetNextItemWidth(TEXT_BASE_WIDTH * 10);
                if (ImGui::InputText("##Temperature", &temperatureInput)) {
                    if (Util::getNumber(&temp, temperatureInput)) {
                        updateFacetProperties = true;
                    }
                }
                if (temp <= 0) {
                    ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"Temperature must be positive (\u212a)");
                }
                else if (updateFacetProperties) {
                    sel->sh.temperature = temp;
                }
#endif

                static double area = 1.0;
                if(sel) area = sel->sh.area;
                ImGui::InputDoubleRightSide(u8"Area [cm\u00b2]", &area);

                static int prof_idx = 0;
                if(sel) prof_idx = sel->sh.profileType;
                if (ImGui::Combo("Profile", &prof_idx,
                                 "None\0Pressure u\0Pressure v\0Incident angle\0Speed distribution\0Orthogonal velocity\0 Tangential velocity\0")) {
                    switch (prof_idx) {
                        case 0:
                            fmt::print("None");
                            break;
                        case 1:
                            fmt::print("Pu");
                            break;
                        case 2:
                            fmt::print("Pv");
                            break;
                        case 3:
                            fmt::print("Angle");
                            break;
                        case 4:
                            fmt::print("Speed");
                            break;
                        case 5:
                            fmt::print("Ortho vel");
                            break;
                        case 6:
                            fmt::print("Tangen vel");
                            break;
                    }
                }
            }
            if (sel == nullptr) ImGui::EndDisabled();
        }


        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
            if(ImGui::Button("<< Sim")){
                mApp->imWnd->globalSet.Toggle();
            }
            ImGui::SameLine();
            {
                if (mApp->worker.IsRunning()) {
                    title = fmt::format("Pause");
                } else if (mApp->worker.globalStatCache.globalHits.nbMCHit > 0) {
                    title = fmt::format("Resume");
                } else {
                    title = fmt::format("Begin");
                }
            }
            if(ImGui::Button(title.c_str())){
                mApp->changedSinceSave = true;
                mApp->StartStopSimulation();
            }
            ImGui::SameLine();
            if(!mApp->worker.IsRunning() && mApp->worker.globalStatCache.globalHits.nbDesorbed > 0)
                ImGui::BeginDisabled();
            if(ImGui::Button("Reset")){
                mApp->changedSinceSave = true;
                mApp->ResetSimulation();
            }
            if(!mApp->worker.IsRunning() && mApp->worker.globalStatCache.globalHits.nbDesorbed > 0)
                ImGui::EndDisabled();

            ImGui::Checkbox("Auto update scene", &mApp->autoFrameMove);
            if(mApp->autoFrameMove){
                ImGui::BeginDisabled();
            }
            if(ImGui::Button("Update")){
                mApp->updateRequested = true;
                //mApp->FrameMove();
            }
            if(mApp->autoFrameMove){
                ImGui::EndDisabled();
            }

            ImVec2 outer_size = ImVec2(std::max(0.0f, ImGui::GetContentRegionAvail().x), 0.0f);
            // for layouting Simulation stats are inside a table/grid
            if (ImGui::BeginTable("simugrid", 2, ImGuiTableFlags_None/*ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                                                 ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                                                 ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                                                 ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                                                 ImGuiTableFlags_Sortable*/, outer_size)) {

                static std::string hit_stat;
                static std::string des_stat;
                bool runningState = mApp->worker.IsRunning();
                if ((mApp->worker.simuTimer.Elapsed() <= 2.0f) && runningState) {
                    hit_stat = "Starting...";
                    des_stat = "Starting...";
                } else {
                    double current_avg = 0.0;
                    if (!runningState) current_avg = mApp->hps_runtotal.avg();
                    else current_avg = (current_avg != 0.0) ? current_avg : mApp->hps.last();

                    hit_stat = fmt::format("{} ({})",
                                           Util::formatInt(mApp->worker.globalStatCache.globalHits.nbMCHit, "hit"),
                                           Util::formatPs(current_avg, "hit"));

                    current_avg = 0.0;
                    if (!runningState) current_avg = mApp->dps_runtotal.avg();
                    else current_avg = (current_avg != 0.0) ? current_avg : mApp->dps.last();

                    des_stat = fmt::format("{} ({})",
                                           Util::formatInt(mApp->worker.globalStatCache.globalHits.nbDesorbed, "des"),
                                           Util::formatPs(current_avg, "des"));
                }


                ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, 0.0f);
                ImGui::TableSetupColumn("field", ImGuiTableColumnFlags_WidthStretch, 0.0f);
                //ImGui::TableHeadersRow();

                static char inputName[128] = "";
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Hits");
                ImGui::TableNextColumn();
                strcpy(inputName, fmt::format("{}", hit_stat).c_str());
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##hit", inputName, IM_ARRAYSIZE(inputName));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Des.");
                ImGui::TableNextColumn();
                strcpy(inputName, fmt::format("{}", des_stat).c_str());
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##des", inputName, IM_ARRAYSIZE(inputName));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Leaks");
                ImGui::TableNextColumn();
                strcpy(inputName, fmt::format("{}", "None").c_str());
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##leak", inputName, IM_ARRAYSIZE(inputName));
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                ImGui::Text("Time");
                if (runningState)
                    strcpy(inputName,
                           fmt::format("Running: {}", Util::formatTime(mApp->worker.simuTimer.Elapsed())).c_str());
                else
                    strcpy(inputName,
                           fmt::format("Stopped: {}", Util::formatTime(mApp->worker.simuTimer.Elapsed())).c_str());
                ImGui::TableNextColumn();
                ImGui::SetNextItemWidth(-FLT_MIN);
                ImGui::InputText("##time", inputName, IM_ARRAYSIZE(inputName));

                ImGui::EndTable();
            }
        }

        try {
            // Facet list
            if (interfGeom->IsLoaded()) {
                // TODO: Colors for moment highlighting

                // Create item list that is sortable etc.
                static ImVector<FacetData> items;
                if (items.Size != interfGeom->GetNbFacet()) {
                    items.resize(static_cast<int>(interfGeom->GetNbFacet()), FacetData());
                    for (int n = 0; n < items.Size; n++) {
                        InterfaceFacet *f = interfGeom->GetFacet(n);
                        FacetData &item = items[n];
                        item.ID = n;
                        item.hits = f->facetHitCache.nbMCHit;
                        item.des = f->facetHitCache.nbDesorbed;
                        item.abs = f->facetHitCache.nbAbsEquiv;
                    }
                }
                else if (mApp->worker.IsRunning()){
                    for (int n = 0; n < items.Size; n++) {
                        InterfaceFacet *f = interfGeom->GetFacet(n);
                        FacetData &item = items[n];
                        item.hits = f->facetHitCache.nbMCHit;
                        item.des = f->facetHitCache.nbDesorbed;
                        item.abs = f->facetHitCache.nbAbsEquiv;
                    }
                }

                static ImGuiTableFlags tFlags =
                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                        /*ImGuiTableFlags_RowBg | */ImGuiTableFlags_BordersOuter |
                        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                        ImGuiTableFlags_Sortable;

                ImVec2 outer_size = ImVec2(0.0f, std::max(ImGui::GetContentRegionAvail().y, TEXT_BASE_HEIGHT * 8.f));
                if (ImGui::BeginTable("facetlist", 4, tFlags, outer_size)) {
                    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_ID);
                    ImGui::TableSetupColumn("Hits", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_Hits);
                    ImGui::TableSetupColumn("Des", ImGuiTableColumnFlags_WidthStretch, 0.0f, FacetDataColumnID_Des);
                    ImGui::TableSetupColumn("Abs", ImGuiTableColumnFlags_WidthStretch, 0.0f, FacetDataColumnID_Abs);
                    ImGui::TableHeadersRow();

                    // Sort our data if sort specs have been changed!
                    if (ImGuiTableSortSpecs *sorts_specs = ImGui::TableGetSortSpecs())
                        if (sorts_specs->SpecsDirty) {
                            FacetData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                            if (items.Size > 1)
                                qsort(&items[0], (size_t) items.Size, sizeof(items[0]),
                                      FacetData::CompareWithSortSpecs);
                            FacetData::s_current_sort_specs = nullptr;
                            sorts_specs->SpecsDirty = false;
                        }

                    // Demonstrate using clipper for large vertical lists
                    ImGuiListClipper clipper;
                    clipper.Begin(items.size());
                    while (clipper.Step()) {
                        for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                            FacetData *item = &items[i];
                            //ImGui::PushID(item->ID);
                            ImGui::TableNextRow();
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("%d", item->ID);
                            ImGui::TableNextColumn();
                            ImGui::Text("%zd", item->hits);
                            ImGui::TableNextColumn();
                            ImGui::Text("%zd", item->des);
                            ImGui::TableNextColumn();
                            ImGui::Text("%g", item->abs);
                            //ImGui::PopID();
                        }
                    }
                    ImGui::EndTable();
                }

            }
        }
        catch (const std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nError while updating facet hits", e.what());
            /*GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);*/
        }
    }
    ImGui::End();
}
