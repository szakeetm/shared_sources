//
// Created by pascal on 2/26/21.
//

#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "ImguiMenu.h"
#include "ImguiAABB.h"

#include "../../src/MolflowGeometry.h"
#include "../../src_shared/Facet_shared.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl.h"
#include "ImguiGlobalSettings.h"

#include <imgui/imgui_internal.h>
#include <sstream>
#include <imgui/IconsFontAwesome5.h>
#include <future>

void ImguiWindow::init() {
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
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
    // - If the file cannot be loaded, the function will return NULL. Please
    // handle those errors in your application (e.g. use an assertion, or display
    // an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored
    // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
    // ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string
    // literal you need to write a double backslash \\ !
    // io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 16.0f);

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);
// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

    // Our state
    show_demo_window = false;
    show_global_settings = false;
    clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);
}

void ImguiWindow::destruct() {
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
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

    const ImGuiTableSortSpecs *FacetData::s_current_sort_specs = NULL;
}

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool *p_open, Geometry *geom) {
    const float PAD = 10.0f;
    static int corner = 0;
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

        char tmp[256];

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
                if (items.Size == 0) {
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

                static ImGuiTableFlags tFlags =
                        ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                        ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
                        ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                        ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                        ImGuiTableFlags_Sortable;

                if (ImGui::BeginTable("facetlist", 4, tFlags)) {
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
                            FacetData::s_current_sort_specs = NULL;
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
static void ShowPerfoPlot(bool *p_open, Interface* mApp) {
    ImGuiIO &io = ImGui::GetIO();

    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(ImVec2(400, 100), ImGuiCond_FirstUseEver);

    static ImGuiWindowFlags flags =
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Perfo", p_open, flags)) {

        // Fill an array of contiguous float values to plot
        // Tip: If your float aren't contiguous but part of a structure, you can pass a pointer to your first float
        // and the sizeof() of your structure in the "stride" parameter.
        static float values[20] = {0.0f};
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
            if (values[values_offset] != values[(values_offset - 1) % IM_ARRAYSIZE(values)])
                values_offset = (values_offset + 1) % IM_ARRAYSIZE(values);
            //phase += 0.10f * values_offset;
            refresh_time = now_time;
        }

        // Plots can display overlay texts
        // (in this example, we will display an average value)
        {
            float average = 0.0f;
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
            ImGui::PlotLines(""/*"Hit/s"*/, values, IM_ARRAYSIZE(values), values_offset, overlay, min_val * 0.95f, max_val * 1.05f,
                             ImVec2(0, 80.0f));
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

        if (show_app_sim_status)
            ShowExampleAppSimpleOverlay(&show_app_sim_status, mApp->worker.GetGeometry());

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
        // ImGui!).
        if ((io.KeyCtrl && io.KeyShift && io.KeyAlt && ImGui::IsKeyDown(SDL_GetScancodeFromKey(SDLK_d))))
            show_demo_window = !show_demo_window;
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair
        // to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("[BETA] _Molflow ImGui Suite_"); // Create a window called "Hello, world!"
            // and append into it.

            ImGui::Checkbox(
                    "Demo Window",
                    &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Global settings", &show_global_settings);
            ImGui::Checkbox("Acceleration Structure", &show_aabb);
            ImGui::Checkbox("Menu bar", &show_app_main_menu_bar);
            ImGui::Checkbox("Sidebar", &show_app_sim_status);
            ImGui::Checkbox("Performance Plot", &show_perfo);

            ImGui::SliderFloat("float", &f, 0.0f,
                               1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3(
                    "clear color",
                    (float *) &clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button")) // Buttons return true when clicked (most
                // widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_perfo) {
            ShowPerfoPlot(&show_perfo, mApp);
        }

        // 3. Show another simple window.
        if (show_global_settings) {
            ShowGlobalSettings(mApp, &show_global_settings, nbProcChanged, recalcOutg, changeDesLimit, nbProc);
            ImGui::End();
        }
        std::promise<int> p;
        static std::future<int> future_int;
        static bool active_prev_state;
        if (!future_int.valid())
            future_int = p.get_future();

        if (show_aabb) {
            static ImguiAABBVisu visu{};
            visu.ShowAABB(mApp, &show_aabb, redrawAabb, rebuildAabb);

            if (rebuildAabb) {
                if (mApp->worker.IsRunning())
                    mApp->StartStopSimulation();

                future_int = std::async(std::launch::async, &SimulationModel::BuildAccelStructure, mApp->worker.model,
                                        &mApp->worker.globState, mApp->worker.model->wp.accel_type,
                                        mApp->worker.model->wp.splitMethod, mApp->worker.model->wp.bvhMaxPrimsInNode);

                active_prev_state = true;
                mApp->wereEvents = true;
            }
            // Evaluate running progress
            if (future_int.wait_for(std::chrono::seconds(0)) != std::future_status::ready) {
                static float progress = 0.0f, load_time = 0.0f;
                ImGui::Loader(progress, load_time);

                active_prev_state = true;
                mApp->wereEvents_imgui = true;
            }
            else if (active_prev_state) {
                redrawAabb = true;
                active_prev_state = false;
                mApp->wereEvents_imgui = true;
            }
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
                    if (ImGui::BeginPopupModal("Error", NULL,
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
        } else if (redrawAabb) {
            mApp->worker.GetGeometry()->BuildGLList();
            redrawAabb = false;
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
        if (ImGui::BeginPopupModal("Error", NULL,
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