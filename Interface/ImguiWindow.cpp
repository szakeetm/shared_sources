//
// Created by pascal on 2/26/21.
//

#include "ImguiWindow.h"

#include "../../src/MolFlow.h"
#include "../../src/MolflowGeometry.h"
#include "../../src_shared/Facet_shared.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl.h"

#include <imgui/imgui_internal.h>
#include <sstream>
#include <imgui/IconsFontAwesome5.h>

void ImguiWindow::init() {
  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
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
    static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_FA, 0 };
    ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
    io.Fonts->AddFontFromFileTTF( FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges );
// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // NULL, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != NULL);

  // Our state
  show_demo_window = false;
  show_another_window = true;
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
  bool show_another_window = true;
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

// Make the UI compact because there are so many fields
static void PushStyleCompact() {
  ImGuiStyle &style = ImGui::GetStyle();
  ImGui::PushStyleVar(
      ImGuiStyleVar_FramePadding,
      ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
  ImGui::PushStyleVar(
      ImGuiStyleVar_ItemSpacing,
      ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact() { ImGui::PopStyleVar(2); }

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char *desc) {
  ImGui::TextDisabled("(?)");
  if (ImGui::IsItemHovered()) {
    ImGui::BeginTooltip();
    ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
    ImGui::TextUnformatted(desc);
    ImGui::PopTextWrapPos();
    ImGui::EndTooltip();
  }
}

static void PlaceAtWindowCenter(const char *str) {
  const std::string btnText = str;
  float font_size = ImGui::GetFontSize() * btnText.size() / 2;
  ImGui::NewLine();
  ImGui::SameLine((ImGui::GetWindowSize().x / 2) - font_size + (font_size / 2));
}

static void PlaceAtRegionCenter(const char *str) {
  // float font_size = ImGui::GetFontSize() * btnText.size() / 2;
  float font_size = ImGui::CalcTextSize(str).x;
  ImGui::NewLine();
  ImGui::SameLine((ImGui::GetContentRegionAvail().x * 0.5f) - (font_size / 2));
}

static void PlaceAtRegionRight(const char *str, bool sameLine) {
  // float font_size = ImGui::GetFontSize() * btnText.size() / 2;
  float font_size = ImGui::CalcTextSize(str).x;
  if (!sameLine)
    ImGui::NewLine();
  ImGui::SameLine(ImGui::GetContentRegionAvail().x -
                  (font_size + ImGui::GetStyle().FramePadding.x * 2));
}

static bool InputRightSide(const char *desc, double *val, const char *format) {
  double tmp = *val;
  ImGui::AlignTextToFramePadding();
  ImGui::Text("%s:", desc);
  {
    // Move to right side
    ImGui::SameLine((ImGui::GetContentRegionAvailWidth()) - 100.0f);
    ImGui::PushItemWidth(100.0f);
    ImGui::PushID(desc);
    ImGui::InputDouble("", val, 0.00f, 0.0f, format);
    ImGui::PopID();
    ImGui::PopItemWidth();
  }

  return *val != tmp; // true if changed
}

// Add spacing of checkbox width
static void AddCheckboxWidthSpacing() {
  ImGui::NewLine();
  ImGui::SameLine(ImGui::GetFrameHeightWithSpacing() +
                  ImGui::GetStyle().FramePadding.y * 4);
}

// Demonstrate creating a simple static window with no decoration
// + a context-menu to choose which corner of the screen to use.
static void ShowExampleAppSimpleOverlay(bool* p_open, Geometry * geom) {
    const float PAD = 10.0f;
    static int corner = 0;
    ImGuiIO &io = ImGui::GetIO();

    const ImGuiViewport *viewport = ImGui::GetMainViewport();
    static bool use_work_area = true;
    ImGui::SetNextWindowPos(use_work_area ?
        ImVec2(viewport->Size.x - viewport->WorkSize.x * 0.25f, viewport->WorkPos.y)
        : viewport->Pos);
    ImGui::SetNextWindowSize(use_work_area ? ImVec2(viewport->WorkSize.x * 0.25f, viewport->WorkSize.y): viewport->Size);
    static ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

    if (ImGui::Begin("Example: Fullscreen window", p_open, flags))
    {
        ImGui::Checkbox("Use work area instead of main area", &use_work_area);
        ImGui::SameLine();
        HelpMarker("Main Area = entire viewport,\nWork Area = entire viewport minus sections used by the main menu bars, task bars etc.\n\nEnable the main-menu bar in Examples menu to see the difference.");

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
                if (ImGui::BeginTable("table1", 4, flags))
                {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Hits", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Des", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableSetupColumn("Abs", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();
                for (int i = 0; i < geom->GetNbFacet(); i++) {
                    int facetId = (int)i;
                    InterfaceFacet *f = geom->GetFacet(facetId);
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("%d", i+1);
                    ImGui::TableNextColumn();
                    ImGui::Text("%zd", f->facetHitCache.hit.nbMCHit);
                    ImGui::TableNextColumn();
                    ImGui::Text("%zd", f->facetHitCache.hit.nbDesorbed);
                    ImGui::TableNextColumn();
                    ImGui::Text("%g", f->facetHitCache.hit.nbAbsEquiv);
                    ImGui::TableNextColumn();

                    /*for (int column = 0; column < 3; column++)
                        {
                            ImGui::TableSetColumnIndex(column);
                            ImGui::Text("%s %d,%d", (column == 2) ? "Stretch" : "Fixed", column, row);
                        }*/

                    }
                    ImGui::EndTable();
                }

            }
        }
        catch(std::exception &e) {
            char errMsg[512];
            sprintf(errMsg, "%s\nError while updating facet hits", e.what());
            /*GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);*/
        }
    }
    ImGui::End();

}

// Note that shortcuts are currently provided for display only
// (future version will add explicit flags to BeginMenu() to request processing
// shortcuts)
static void ShowExampleMenuFile() {
  ImGui::MenuItem("(demo menu)", NULL, false, false);
  if (ImGui::MenuItem("New")) {
  }
  if (ImGui::MenuItem("Open", "Ctrl+O")) {
  }
  if (ImGui::BeginMenu("Open Recent")) {
    ImGui::MenuItem("fish_hat.c");
    ImGui::MenuItem("fish_hat.inl");
    ImGui::MenuItem("fish_hat.h");
    if (ImGui::BeginMenu("More..")) {
      ImGui::MenuItem("Hello");
      ImGui::MenuItem("Sailor");
      if (ImGui::BeginMenu("Recurse..")) {
        ShowExampleMenuFile();
        ImGui::EndMenu();
      }
      ImGui::EndMenu();
    }
    ImGui::EndMenu();
  }
  if (ImGui::MenuItem("Save", "Ctrl+S")) {
  }
  if (ImGui::MenuItem("Save As..")) {
  }

  ImGui::Separator();
  if (ImGui::BeginMenu("Options")) {
    static bool enabled = true;
    ImGui::MenuItem("Enabled", "", &enabled);
    ImGui::BeginChild("child", ImVec2(0, 60), true);
    for (int i = 0; i < 10; i++)
      ImGui::Text("Scrolling Text %d", i);
    ImGui::EndChild();
    static float f = 0.5f;
    static int n = 0;
    ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
    ImGui::InputFloat("Input", &f, 0.1f);
    ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Colors")) {
    float sz = ImGui::GetTextLineHeight();
    for (int i = 0; i < ImGuiCol_COUNT; i++) {
      const char *name = ImGui::GetStyleColorName((ImGuiCol)i);
      ImVec2 p = ImGui::GetCursorScreenPos();
      ImGui::GetWindowDrawList()->AddRectFilled(
          p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
      ImGui::Dummy(ImVec2(sz, sz));
      ImGui::SameLine();
      ImGui::MenuItem(name);
    }
    ImGui::EndMenu();
  }

  // Here we demonstrate appending again to the "Options" menu (which we already
  // created above) Of course in this demo it is a little bit silly that this
  // function calls BeginMenu("Options") twice. In a real code-base using it
  // would make senses to use this feature from very different code locations.
  if (ImGui::BeginMenu("Options")) // <-- Append!
  {
    static bool b = true;
    ImGui::Checkbox("SomeOption", &b);
    ImGui::EndMenu();
  }

  if (ImGui::BeginMenu("Disabled", false)) // Disabled
  {
    IM_ASSERT(0);
  }
  if (ImGui::MenuItem("Checked", NULL, true)) {
  }
  if (ImGui::MenuItem("Quit", "Alt+F4")) {
  }
}

//-----------------------------------------------------------------------------
// [SECTION] Example App: Main Menu Bar / ShowExampleAppMainMenuBar()
//-----------------------------------------------------------------------------
// - ShowExampleAppMainMenuBar()
// - ShowExampleMenuFile()
//-----------------------------------------------------------------------------

// Demonstrate creating a "main" fullscreen menu bar and populating it.
// Note the difference between BeginMainMenuBar() and BeginMenuBar():
// - BeginMenuBar() = menu-bar inside current window (which needs the
// ImGuiWindowFlags_MenuBar flag!)
// - BeginMainMenuBar() = helper to create menu-bar-sized window at the top of
// the main viewport + call BeginMenuBar() into it.
static void ShowExampleAppMainMenuBar() {

    //ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.f, 16.f));
    //ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, ImGui::GetStyle().ItemSpacing.y + 16.0f));
    if (ImGui::BeginMainMenuBar()) {
        ImGui::AlignTextToFramePadding();
        if (ImGui::BeginMenu("File")) {
      ShowExampleMenuFile();
      ImGui::EndMenu();
    }
        if (ImGui::BeginMenu("Selection")) {
      if (ImGui::MenuItem("Smart Select facets...", "ALT+S")) {
      }
        ImGui::Separator();
        if (ImGui::MenuItem("Select All Facets", "CTRL+A")) {
      } // Disabled item
      if (ImGui::MenuItem("Select by Facet Number...", "ALT+N")) {
      }
      if (ImGui::MenuItem("Select Sticking", "")) {
      }
      if (ImGui::MenuItem("Select Transparent", "")) {
      }
        if (ImGui::MenuItem("...", "", false, false)) {
        } // Disabled item
      ImGui::EndMenu();
    }
    if (ImGui::BeginMenu(ICON_FA_TOOLS "  Tools")) {
      if (ImGui::MenuItem("...", "", false, false)) {
      } // Disabled item
      ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Facet")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Vertex")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("View")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Test")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("Time")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }
      if (ImGui::BeginMenu("About")) {
          if (ImGui::MenuItem("...", "", false, false)) {
          } // Disabled item
          ImGui::EndMenu();
      }

        ImGui::EndMainMenuBar();
  }
    //ImGui::PopStyleVar(1);

}

void ImguiWindow::ProcessControlTable(MolFlow *mApp) {
  ImGui::Text("Process control");
  static ImGuiTableFlags flags =
      ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
      ImGuiTableFlags_RowBg | ImGuiTableFlags_BordersOuter |
      ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
      ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;

  const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("0").x;
  const float TEXT_BASE_HEIGHT = ImGui::GetTextLineHeightWithSpacing();
  // When using ScrollX or ScrollY we need to specify a size for our table
  // container! Otherwise by default the table will fit all available space,
  // like a BeginChild() call.
  ImVec2 outer_size = ImVec2(0.0f, TEXT_BASE_HEIGHT * 8);
  if (ImGui::BeginTable("procTable", 5, flags, outer_size)) {
    ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
    ImGui::TableSetupColumn("#",  ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 10.f);
    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Mem Usage", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Mem Peak", ImGuiTableColumnFlags_WidthFixed);
    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);
    ImGui::TableHeadersRow();

    size_t states[MAX_PROCESS];
    std::vector<std::string> statusStrings(MAX_PROCESS);
    memset(states, 0, MAX_PROCESS * sizeof(int));
    mApp->worker.GetProcStatus(states, statusStrings);

    ProcComm procInfo;
    mApp->worker.GetProcStatus(procInfo);

    ImGui::TableNextRow();

    // Interface
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
    size_t currPid = GetCurrentProcessId();
    double memDenom = (1024.0 * 1024.0);
#else
    size_t currPid = getpid();
    double memDenom = (1024.0);
#endif
    PROCESS_INFO parentInfo{};
    GetProcInfo(currPid, &parentInfo);
    ImGui::TableSetColumnIndex(0);
    ImGui::Text("Interface");
    ImGui::TableSetColumnIndex(1);
    ImGui::Text("%zd", currPid);
    ImGui::TableSetColumnIndex(2);
    ImGui::Text("%.0f MB", (double)parentInfo.mem_use / memDenom);
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%.0f MB", (double)parentInfo.mem_peak / memDenom);
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("");

    size_t i = 1;
    // Demonstrate using clipper for large vertical lists
    ImGuiListClipper clipper;
    clipper.Begin(procInfo.subProcInfo.size());
    while (clipper.Step()) {
      for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
        i = row;
        auto &proc = procInfo.subProcInfo[i];
        DWORD pid = proc.procId;
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Thread %zu", i + 1);
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%u", pid);

        PROCESS_INFO pInfo = proc.runtimeInfo;
        // GetProcInfo(pid, &pInfo);

        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.0f MB", (double)pInfo.mem_use / memDenom);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.0f MB", (double)pInfo.mem_peak / memDenom);

        // State/Status
        std::stringstream tmp_ss;
        // tmp_ss << "[" << prStates[states[i-1]] << "] " << statusStrings[i-1];
        tmp_ss << "[" << prStates[proc.slaveState] << "] " << proc.statusString;

        ImGui::TableSetColumnIndex(4);
        ImGui::Text("%s", tmp_ss.str().c_str());
        ++i;
      }
    }
    ImGui::EndTable();
  }
}

void ImguiWindow::renderSingle() {
  ImGuiIO &io = ImGui::GetIO(); (void)io;

  MolFlow *mApp = (MolFlow *)app;
  if (mApp) {

    bool nbProcChanged = false;
    bool recalcOutg = false;
    bool changeDesLimit = false;
    static int nbProc = mApp->worker.GetProcNumber();
    static bool show_app_main_menu_bar = false;
    static bool show_app_sim_status = false;

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL2_NewFrame();
    ImGui_ImplSDL2_NewFrame(app->mainScreen);
    ImGui::NewFrame();

    if (show_app_main_menu_bar)
      ShowExampleAppMainMenuBar();

    if (show_app_sim_status)
        ShowExampleAppSimpleOverlay(&show_app_sim_status, mApp->worker.GetGeometry());

    // 1. Show the big demo window (Most of the sample code is in
    // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
    // ImGui!).
    if (show_demo_window)
      ImGui::ShowDemoWindow(&show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair
    // to created a named window.
    {
      static float f = 0.0f;
      static int counter = 0;

      ImGui::Begin("Hello, world!"); // Create a window called "Hello, world!"
                                     // and append into it.

      ImGui::Text("This is some useful text."); // Display some text (you can
                                                // use a format strings too)
      ImGui::Checkbox(
          "Demo Window",
          &show_demo_window); // Edit bools storing our window open/close state
      ImGui::Checkbox("Another Window", &show_another_window);
      ImGui::Checkbox("Menu bar", &show_app_main_menu_bar);
      ImGui::Checkbox("Sidebar", &show_app_sim_status);

      ImGui::SliderFloat("float", &f, 0.0f,
                         1.0f); // Edit 1 float using a slider from 0.0f to 1.0f
      ImGui::ColorEdit3(
          "clear color",
          (float *)&clear_color); // Edit 3 floats representing a color

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
    if (show_another_window) {
      if (mApp) {

        ImGui::PushStyleVar(
            ImGuiStyleVar_WindowMinSize,
            ImVec2(800.f,
                   0)); // Lift normal size constraint, however the presence of
                        // a menu-bar will give us the minimum height we want.

        ImGui::Begin(
            "Global settings", &show_another_window,
            ImGuiWindowFlags_NoSavedSettings); // Pass a pointer to our bool
                                               // variable (the window will have
                                               // a closing button that will
                                               // clear the bool when clicked)
        ImGui::PopStyleVar(1);

        float gasMass = 2.4;
        if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersInnerV)) {
          // PushStyleCompact();
          // ImGui::TableNextColumn();
          // ImGui::PushID(0);
          // ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for
          // wrong text baseline propagation
          ImGui::TableSetupColumn("Global settings");
          ImGui::TableSetupColumn("Simulation settings");
          ImGui::TableHeadersRow();
          ImGui::TableNextRow();
          ImGui::TableSetColumnIndex(0);
          ImGui::PushItemWidth(100);
          ImGui::InputDouble("Autosave frequency (minutes)",
                             &mApp->autoSaveFrequency, 0.00f, 0.0f, "%.1f");
          ImGui::PopItemWidth();
          ImGui::Checkbox(
              "Autosave only when simulation is running",
              reinterpret_cast<bool *>(
                  &mApp->autoSaveSimuOnly)); // Edit bools storing our window
                                             // open/close state
          ImGui::Checkbox(
              "Use .zip as default extension (otherwise .xml)",
              reinterpret_cast<bool *>(
                  &mApp->compressSavedFiles)); // Edit bools storing our window
                                               // open/close state
          ImGui::Checkbox(
              "Check for updates at startup",
              reinterpret_cast<bool *>(
                  &mApp->updateRequested)); // Edit bools storing our window
                                            // open/close state
          ImGui::Checkbox(
              "Auto refresh formulas",
              reinterpret_cast<bool *>(
                  &mApp->autoUpdateFormulas)); // Edit bools storing our window
                                               // open/close state
          ImGui::Checkbox("Anti-Aliasing",
                          &mApp->antiAliasing); // Edit bools storing our window
                                                // open/close state
          ImGui::Checkbox(
              "White Background",
              &mApp->whiteBg); // Edit bools storing our window open/close state
          ImGui::Checkbox("Left-handed coord. system",
                          &mApp->leftHandedView); // Edit bools storing our
                                                  // window open/close state
          ImGui::Checkbox(
              "Highlight non-planar facets",
              &mApp->highlightNonplanarFacets); // Edit bools storing our window
                                                // open/close state
          ImGui::Checkbox("Highlight selected facets",
                          &mApp->highlightSelection); // Edit bools storing our
                                                      // window open/close state
          ImGui::Checkbox("Use old XML format",
                          &mApp->useOldXMLFormat); // Edit bools storing our
                                                   // window open/close state
          ImGui::TableNextColumn();
          // ImGui::PushID(1);
          // ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for
          // wrong text baseline propagation ImGui::TableSetupColumn("Simulation
          // settings"); ImGui::TableHeadersRow();
          ImGui::PushItemWidth(100);

          /* --- Simu settings ---*/
          static bool simChanged = false;
          static double gasMass = mApp->worker.model.wp.gasMass;
          static bool enableDecay = mApp->worker.model.wp.enableDecay;
          static double halfLife = mApp->worker.model.wp.halfLife;
          static bool lowFluxMode = mApp->worker.model.otfParams.lowFluxMode;
          static double lowFluxCutoff =
              mApp->worker.model.otfParams.lowFluxCutoff;

          simChanged =
              InputRightSide("Gas molecular mass (g/mol)", &gasMass, "%g");
          simChanged = ImGui::Checkbox("", &enableDecay);
          if (!enableDecay) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  IM_COL32(217, 217, 217, 255));
          }
          ImGui::SameLine();
          simChanged = InputRightSide("Gas half life (s)", &halfLife, "%g");

          if (!enableDecay) {
            ImGui::PopStyleColor();
            ImGui::PopItemFlag();
          }

          ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
          ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(217, 217, 217, 255));

          // Use tmp var to multiply by 10
          double outgRate10 =
              mApp->worker.model.wp.finalOutgassingRate_Pa_m3_sec * 10.00;
          InputRightSide("Final outgassing rate (mbar*l/sec)", &outgRate10,
                         "%.4g"); // 10: conversion Pa*m3/sec -> mbar*l/s
          InputRightSide("Final outgassing rate (1/sec)",
                         &mApp->worker.model.wp.finalOutgassingRate,
                         "%.4g"); // In molecules/sec

          {
            char tmp[64];
            sprintf(tmp, "Tot.des. molecules [0 to %g s]",
                    mApp->worker.model.wp.latestMoment);
            InputRightSide(tmp, &mApp->worker.model.wp.totalDesorbedMolecules,
                           "%.4g");
          }

          ImGui::PopStyleColor();
          ImGui::PopItemFlag();
          //***********************

          {
            const std::string btnText = "Recalc. outgassing";
            PlaceAtRegionRight(btnText.c_str(), false);
          }

          if (ImGui::Button("Recalc. outgassing")) // Edit bools storing our
                                                   // window open/close state
            recalcOutg = true;
          simChanged = ImGui::Checkbox(
              "Enable low flux mode",
              &lowFluxMode); // Edit bools storing our window open/close state
          ImGui::SameLine();
          HelpMarker(
              "Low flux mode helps to gain more statistics on low pressure "
              "parts of the system, at the expense\n"
              "of higher pressure parts. If a traced particle reflects from a "
              "high sticking factor surface, regardless of that probability,\n"
              "a reflected test particle representing a reduced flux will "
              "still be traced. Therefore test particles can reach low flux "
              "areas more easily, but\n"
              "at the same time tracing a test particle takes longer. The "
              "cutoff ratio defines what ratio of the originally generated "
              "flux\n"
              "can be neglected. If, for example, it is 0.001, then, when "
              "after subsequent reflections the test particle carries less "
              "than 0.1%\n"
              "of the original flux, it will be eliminated. A good advice is "
              "that if you'd like to see pressure across N orders of "
              "magnitude, set it to 1E-N");
          if (!lowFluxMode) {
            ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
            ImGui::PushStyleColor(ImGuiCol_FrameBg,
                                  IM_COL32(217, 217, 217, 255));
          }
          simChanged = InputRightSide(
              "Cutoff ratio", &lowFluxCutoff,
              "%.2e"); // Edit bools storing our window open/close state
          if (!lowFluxMode) {
            ImGui::PopStyleColor();
            ImGui::PopItemFlag();
          }

          {
            PlaceAtRegionCenter("Apply above settings");
            if (ImGui::Button("Apply above settings")) {
              simChanged = false;
              mApp->worker.model.wp.gasMass = gasMass;
              mApp->worker.model.wp.enableDecay = enableDecay;
              mApp->worker.model.wp.halfLife = halfLife;
              mApp->worker.model.otfParams.lowFluxMode = lowFluxMode;
              mApp->worker.model.otfParams.lowFluxCutoff = lowFluxCutoff;
            }
          }
          ImGui::PopItemWidth();
          // PopStyleCompact();
          ImGui::EndTable();
        }

        ProcessControlTable(mApp);

        ImGui::Text("Number of CPU cores:     %zd", mApp->numCPU);
        ImGui::Text("Number of subprocesses:  ");
        ImGui::SameLine();
        /*bool nbProcChanged = false;
        int nbProc = mApp->worker.GetProcNumber();*/
        ImGui::SetNextItemWidth(50.0f);
        ImGui::DragInt("", &nbProc, 1, 0, MAX_PROCESS, "%d",
                       ImGuiSliderFlags_AlwaysClamp);

        ImGui::SameLine();
        if (ImGui::Button("Apply and restart processes"))
          nbProcChanged = true;
        {
          PlaceAtRegionRight("Change MAX desorbed molecules", true);
          if (ImGui::Button("Change MAX desorbed molecules"))
            ImGui::OpenPopup("Edit MAX");
        }
        // Always center this window when appearing
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                                ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopupModal("Edit MAX", NULL,
                                   ImGuiWindowFlags_AlwaysAutoResize)) {
          // static bool initMax = false;
          static double maxDes = mApp->worker.model.otfParams.desorptionLimit;
          ;
          /*if(!initMax) {
              maxDes = mApp->worker.model.otfParams.desorptionLimit; // use
          double function to allow exponential format initMax = true;
          }*/
          ImGui::InputDouble("Desorption max (0=>endless)", &maxDes, 0.0f, 0.0f,
                             "%.0f");
          ImGui::Separator();

          // static int unused_i = 0;
          // ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

          /*static bool dont_ask_me_next_time = false;
          ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
          ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
          ImGui::PopStyleVar();*/

          if (ImGui::Button("OK", ImVec2(120, 0))) {
            mApp->worker.model.otfParams.desorptionLimit = maxDes;
            // initMax = false;
            changeDesLimit = true;
            ImGui::CloseCurrentPopup();
          }
          ImGui::SetItemDefaultFocus();
          ImGui::SameLine();
          if (ImGui::Button("Cancel", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
          }
          ImGui::EndPopup();
        }
      }
      ImGui::End();
    }

    // Rendering
    ImGui::Render();
    glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
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