//
// Created by pascal on 2/26/21.
//

#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "ImguiMenu.h"

#include "../../src/MolflowGeometry.h"
#include "Facet_shared.h"
#include "../../src/Interface/Viewer3DSettings.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl.h"
#include "ImguiGlobalSettings.h"

#include <imgui/imgui_internal.h>
#include <imgui/IconsFontAwesome5.h>
#include <future>
#include <implot/implot.h>
#include <Helper/FormatHelper.h>

void ImguiWindow::init() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    // Setup Dear ImGui style
    // ImGui::StyleColorsDark();
    // ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(app->mainScreen, app->mainContext);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can
    // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
    // them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
    // need to select the font among multiple.
    // - If the file cannot be loaded, the function will return nullptr. Please
    // handle those errors in your application (e.g. use an assertion, or display
    // an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored
    // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    static const ImWchar sym_ranges[] = {0x2000, 0x3000, 0};
    ImFontConfig sym_config;
    sym_config.MergeMode = true;
    sym_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 16.0f, &sym_config, sym_ranges); // vector arrow

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);

    io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 14.0f);
    io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 14.0f, &sym_config, sym_ranges); // vector arrow
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 14.0f, &icons_config, icons_ranges);

    /*io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 16.0f, &sym_config, sym_ranges);*/

// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

    show_demo_window = false;
    show_global_settings = false;
    show_app_main_menu_bar = false;
    show_app_sim_status = false;
    show_perfo = false;

    start_time = ImGui::GetTime();
}

void ImguiWindow::destruct() {
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
}

/*void ImguiWindow::render() {
  ImGuiIO &io = ImGui::GetIO();
  (void)io;

  // Our state
  bool show_demo_window = false;
  bool show_global_settings = true;
  ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

  // Main loop
  bool done = false;
  while (!done) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL2_ProcessEvent(&event);
      if (event.type == SDL_QUIT)
        done = true;
      if (event.type == SDL_WINDOWEVENT &&
          event.window.event == SDL_WINDOWEVENT_CLOSE &&
          event.window.windowID == SDL_GetWindowID(app->mainScreen))
        done = true;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(app->mainScreen);
    ImGui::NewFrame();

    // Show contents

    // Rendering
    ImGui::Render();
    glViewport((int)io.DisplaySize.x * 0.25, (int)io.DisplaySize.y * 0.25,
               (int)io.DisplaySize.x / 2.0, (int)io.DisplaySize.y / 2.0);
    // glClearColor(clear_color.x * clear_color.w, clear_color.y *
    // clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    // glClear(GL_COLOR_BUFFER_BIT);
    // glUseProgram(0); // You may want this if using this code in an OpenGL 3+
    // context where shaders may be bound
    ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
    SDL_GL_SwapWindow(app->mainScreen);
  }
}*/

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

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void
ShowExampleAppSimpleOverlay(bool *p_open, MolFlow *mApp, Geometry *geom, bool *show_global, bool *newViewer) {
    const float PAD = 10.0f;
    static int corner = 0;
    static float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
    static float TEXT_BASE_WIDTH = ImGui::CalcTextSize("A").x;

    ImGuiIO &io = ImGui::GetIO();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    static bool use_work_area = true;
    ImGui::SetNextWindowPos(use_work_area ?
                            ImVec2(viewport->Size.x - viewport->WorkSize.x * 0.25f, viewport->WorkPos.y)
                                          : viewport->Pos);
    ImGui::SetNextWindowSize(
            use_work_area ? ImVec2(viewport->WorkSize.x * 0.25f, viewport->WorkSize.y) : viewport->Size);
    static ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Example: Fullscreen window", p_open, flags)) {
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

            if (p_open && ImGui::Button("Close this window"))
                *p_open = false;
        }

        if (ImGui::CollapsingHeader("3D Viewer settings", ImGuiTreeNodeFlags_DefaultOpen)) {
            auto curViewer = mApp->curViewer;
            auto viewer = mApp->viewer[curViewer];
            if (ImGui::BeginTable("table_3dviewer", 3, ImGuiTableFlags_None)) {
                /*ImGui::TableSetupColumn("col1");
                ImGui::TableSetupColumn("col2");
                ImGui::TableSetupColumn("col3");
                ImGui::TableHeadersRow();*/
                ImGui::TableNextRow();
                {
                    ImGui::TableNextColumn();
                    ImGui::Checkbox("Rules", &viewer->showRule);
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
                        *newViewer = true;
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
        if(geom->GetNbSelectedFacets() > 1) {
            title = fmt::format("Selected Facet ({} selected)", geom->GetNbSelectedFacets());
        }
        else if(geom->GetNbSelectedFacets() == 1){
            title = fmt::format("Selected Facet (#{})", geom->GetSelectedFacets().front());
        }
        else {
            title = fmt::format("Selected Facet (none)");
        }
        if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.35f);
            ImGui::PushItemWidth(ImGui::GetContentRegionAvailWidth()*0.25f);
            size_t selected_facet_id = geom->GetNbSelectedFacets() ? geom->GetSelectedFacets().front() : 0;
            auto sel = geom->GetNbSelectedFacets() ? geom->GetFacet(selected_facet_id) : nullptr;
            if (ImGui::TreeNodeEx("Particles in", ImGuiTreeNodeFlags_DefaultOpen)) {
                static int des_idx = 0;
                if(sel) des_idx = sel->sh.desorbType;
                if (ImGui::Combo("Desorption", &des_idx, "None\0Uniform\0Cosine\0Cosine\u207f\0Recorded\0")) {
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
                            fmt::print("cos \u207f");
                            break;
                        case 4:
                            fmt::print("rec");
                            break;
                    }
                }

                static bool use_og_area = false;
                bool use_og = !use_og_area;
                static double og = 1.0;
                static double og_area = 1.0;

                if(sel) og = sel->sh.outgassing;
                if (ImGui::Checkbox("Outgassing [mbar\u00b7l/s]", &use_og)) {
                    use_og_area = !use_og;
                }
                ImGui::SameLine();
                ImGui::InputDouble("##in", &og);
                ImGui::Checkbox(u8"Outg/area [mbar\u00b7l/s/cm\u00b2]", &use_og_area);
                ImGui::SameLine();
                ImGui::InputDouble("##ina", &og);
                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Particles out", ImGuiTreeNodeFlags_DefaultOpen)) {
                static double sf = 1.0;
                static double ps = 1.0;
                if(sel) sf = sel->sh.sticking;
                ImGui::InputRightSide("Sticking factor", &sf);
                ImGui::InputRightSide("Pumping speed [l/s]", &ps);
                ImGui::TreePop();
            }
            ImGui::PopItemWidth();
            {
                static int sides_idx = 0;
                if(sel) sides_idx = sel->sh.is2sided;
                if (ImGui::Combo("Sides", &sides_idx, "1 Sided\0 2 Sided\0")) {
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
                if(sel) opacity = sel->sh.opacity;
                ImGui::InputRightSide("Opacity", &opacity);

                static double temp = 1.0;
                if(sel) temp = sel->sh.temperature;
                ImGui::InputRightSide("Temperature [\u00b0\u212a]", &temp);

                static double area = 1.0;
                if(sel) area = sel->sh.area;
                ImGui::InputRightSide("Area [cm\u00b2]", &area);

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
            ImGui::PopItemWidth();
        }


        if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
            if(ImGui::Button("<< Sim")){
                *show_global = !*show_global;
            }
            ImGui::SameLine();
            {
                if (mApp->worker.IsRunning()) {
                    title = fmt::format("Pause");
                } else if (mApp->worker.globalHitCache.globalHits.nbMCHit > 0) {
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
            if(!mApp->worker.IsRunning() && mApp->worker.globalHitCache.globalHits.nbDesorbed > 0)
                ImGui::BeginDisabled();
            if(ImGui::Button("Reset")){
                mApp->changedSinceSave = true;
                mApp->ResetSimulation();
            }
            if(!mApp->worker.IsRunning() && mApp->worker.globalHitCache.globalHits.nbDesorbed > 0)
                ImGui::EndDisabled();

            ImGui::Checkbox("Auto update scene", &mApp->autoFrameMove);
            if(mApp->autoFrameMove){
                ImGui::BeginDisabled();
            }
            if(ImGui::Button("Update")){
                mApp->updateRequested = true;
                mApp->FrameMove();
            }
            if(mApp->autoFrameMove){
                ImGui::EndDisabled();
            }

            ImVec2 outer_size = ImVec2(std::max(0.0f, ImGui::GetContentRegionAvail().x), 0.0f);
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
                                           Util::formatInt(mApp->worker.globalHitCache.globalHits.nbMCHit, "hit"),
                                           Util::formatPs(current_avg, "hit"));

                    current_avg = 0.0;
                    if (!runningState) current_avg = mApp->dps_runtotal.avg();
                    else current_avg = (current_avg != 0.0) ? current_avg : mApp->dps.last();

                    des_stat = fmt::format("{} ({})",
                                           Util::formatInt(mApp->worker.globalHitCache.globalHits.nbDesorbed, "des"),
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
            if (geom->IsLoaded()) {

                /*if (worker.displayedMoment == 0) {
                    int colors[] = { COLOR_BLACK, COLOR_BLACK, COLOR_BLACK, COLOR_BLACK };
                    facetList->SetColumnColors(colors);
                }
                else
                {
                    int colors[] = { COLOR_BLACK, COLOR_BLUE, COLOR_BLUE, COLOR_BLUE };
                    facetList->SetColumnColors(colors);
                }*/

                // Create item list
                static ImVector<FacetData> items;
                if (items.Size != geom->GetNbFacet()) {
                    items.resize(geom->GetNbFacet(), FacetData());
                    for (int n = 0; n < items.Size; n++) {
                        InterfaceFacet *f = geom->GetFacet(n);
                        FacetData &item = items[n];
                        item.ID = n;
                        item.hits = f->facetHitCache.nbMCHit;
                        item.des = f->facetHitCache.nbDesorbed;
                        item.abs = f->facetHitCache.nbAbsEquiv;
                    }
                }
                else if (mApp->worker.IsRunning()){
                    for (int n = 0; n < items.Size; n++) {
                        InterfaceFacet *f = geom->GetFacet(n);
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
        catch (std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nError while updating facet hits", e.what());
            /*GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);*/
        }
    }
    ImGui::End();
}

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowPerfoPlot(bool *p_open, Interface *mApp) {
    ImGuiIO &io = ImGui::GetIO();

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 400), ImGuiCond_FirstUseEver);

    static ImGuiWindowFlags flags =/*
            ImGuiWindowFlags_AlwaysAutoResize |*/
            ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Perfo", p_open, flags)) {

        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.
        static float values[20] = {0.0f};
        static float tvalues[20] = {0.0f};
        static int values_offset = 0;
        static double refresh_time = 0.0;
        if (!true || refresh_time == 0.0) // force
            refresh_time = ImGui::GetTime();
        auto now_time = ImGui::GetTime();
        if (mApp->worker.IsRunning() && difftime(now_time, refresh_time) > 1.0 &&
            mApp->hps.eventsAtTime.size() >= 2) // Create data at fixed 60 Hz rate for the demo
        {
            //static float phase = 0.0f;
            values[values_offset] = mApp->hps.avg();
            tvalues[values_offset] = now_time;
            if (values[values_offset] != values[(values_offset - 1) % IM_ARRAYSIZE(values)])
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            //phase += 0.10f * values_offset;
            refresh_time = now_time;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            /*float average = 0.0f;
            for (float value: values)
                average += value;
            average /= (float) IM_ARRAYSIZE(values);

            float max_val = values[0];
            float min_val = values[0];
            for (int i = 1; i < IM_ARRAYSIZE(values); ++i) {
                if (values[i] > max_val) {
                    max_val = values[i];
                }
                if (values[i] < min_val) {
                    min_val = values[i];
                }
            }
            char overlay[32];
            sprintf(overlay, "avg %f hit/s", average);
            ImGui::PlotLines(""*//*"Hit/s"*//*, values, IM_ARRAYSIZE(values), values_offset, overlay, min_val * 0.95f, max_val * 1.05f,
                             ImVec2(0, 80.0f));*/
            if (ImPlot::BeginPlot("##Perfo", "time (s)", "performance (hit/s)", ImVec2(-1, -1), ImPlotFlags_AntiAliased,
                                  ImPlotAxisFlags_AutoFit | ImPlotAxisFlags_Time, ImPlotAxisFlags_AutoFit)) {
                ImPlot::PlotLine("Simulation", tvalues, values, IM_ARRAYSIZE(values), values_offset);
                ImPlot::EndPlot();
            }
        }
    }
    ImGui::End();
}

void ImguiWindow::renderSingle() {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

    MolFlow *mApp = (MolFlow *) app;
    if (mApp) {
        bool nbProcChanged = false;
        bool recalcOutg = false;
        bool changeDesLimit = false;
        bool redrawAabb = false;
        bool rebuildAabb = false;
        static int nbProc = mApp->worker.GetProcNumber();


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(/*app->mainScreen*/);
        ImGui::NewFrame();

        if (show_app_main_menu_bar)
            ShowAppMainMenuBar();

        static bool open_viewer_window = false;
        if (show_app_sim_status)
            ShowExampleAppSimpleOverlay(&show_app_sim_status, mApp, mApp->worker.GetGeometry(), &show_global_settings,
                                        &open_viewer_window);

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
        // ImGui!).
        if ((io.KeyCtrl && io.KeyShift && io.KeyAlt && ImGui::IsKeyDown(SDL_GetScancodeFromKey(SDLK_d))))
            show_demo_window = !show_demo_window;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show Molflow x ImGui Hub window
        {
            ImGui::Begin("[BETA] _Molflow ImGui Suite_"); // Create a window called "Hello, world!"
            // and append into it.

            ImGui::Checkbox(
                    "Demo Window",
                    &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Global settings", &show_global_settings);
            ImGui::Checkbox("Menu bar", &show_app_main_menu_bar);
            ImGui::Checkbox("Sidebar", &show_app_sim_status);
            ImGui::Checkbox("Performance Plot", &show_perfo);

            ImGui::Text("Avg %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            auto now_time = ImGui::GetTime();
            ImGui::Text("Application time %.3f s [%.3f s]",
                        ImGui::GetTime(), difftime(now_time, start_time));
            ImGui::End();
        }

        // 3. Show window plotting the simulation performance
        if (show_perfo) {
            ShowPerfoPlot(&show_perfo, mApp);
        }

        // 3. Show global settings
        if (show_global_settings) {
            ShowGlobalSettings(mApp, &show_global_settings, nbProcChanged, recalcOutg, changeDesLimit, nbProc);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        // SDL_GL_SwapWindow(app->mainScreen);

        // Handle button events at the end, because some functions call GL Repaint,
        // which doesnt work well with ImGui if frame is not finalized
        if (nbProcChanged) {
            restartProc(nbProc, mApp);
        } else if (recalcOutg) {
            if (mApp->AskToReset()) {
                try {
                    mApp->worker.RealReload();
                } catch (std::exception &e) {
                    if (ImGui::BeginPopupModal("Error", nullptr,
                                               ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Recalculation failed: Couldn't reload Worker:\n%s",
                                    e.what());
                        ImGui::Separator();

                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SetItemDefaultFocus();
                        ImGui::EndPopup();
                    }
                }
            }
        } else if (changeDesLimit) {
            mApp->worker.ChangeSimuParams(); // Sync with subprocesses
        }
        if(open_viewer_window){
            open_viewer_window = false;
            if (!mApp->viewer3DSettings)
                mApp->viewer3DSettings = new Viewer3DSettings();
            mApp->viewer3DSettings->SetVisible(!mApp->viewer3DSettings->IsVisible());
            mApp->viewer3DSettings->Reposition();
            auto curViewer = mApp->curViewer;
            auto viewer = mApp->viewer[curViewer];
            mApp->viewer3DSettings->Refresh(mApp->worker.GetGeometry(), viewer);
        }
    }
}

/**
 * \brief Function to apply changes to the number of processes.
 */
void ImguiWindow::restartProc(int nbProc, MolFlow *mApp) {
    try {
        mApp->worker.Stop_Public();
        mApp->worker.SetProcNumber(nbProc);
        mApp->worker.RealReload(true);
        mApp->SaveConfig();
    } catch (Error &e) {
        if (ImGui::BeginPopupModal("Error", nullptr,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("%s", e.what());
            ImGui::Separator();

            if (ImGui::Button("OK", ImVec2(120, 0))) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::SetItemDefaultFocus();
            ImGui::EndPopup();
        }
    }
}