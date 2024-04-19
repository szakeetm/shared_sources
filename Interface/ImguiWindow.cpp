

#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif // IMGUI_DEFINE_MATH_OPERATORS
#include "imgui/imgui.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "ImguiMenu.h"
#include <imgui/imgui_internal.h>

#if defined(MOLFLOW)
#include "../../src/MolflowGeometry.h"
#include "../../src/versionId.h"
#else
#include "../../src/SynradGeometry.h"
#endif
#include "Facet_shared.h"
#include "../../src/Interface/Viewer3DSettings.h"
#include "imgui/imgui_impl_opengl2.h"
#include "imgui/imgui_impl_sdl2.h"
#include "ImguiGlobalSettings.h"
#include "ImguiPerformancePlot.h"
#include "ImguiSidebar.h"

#include <imgui/IconsFontAwesome5.h>
#include <future>
#include <implot/implot.h>
#include <Helper/FormatHelper.h>
#include "imgui_stdlib/imgui_stdlib.h"

// Varius toggle functions for individual window components
bool ImguiWindow::ToggleMainHub(){
    show_main_hub = !show_main_hub;
    return show_main_hub;
}
bool ImguiWindow::ToggleMainMenu(){
    show_app_main_menu_bar = !show_app_main_menu_bar;
    return show_app_main_menu_bar;
}
bool ImguiWindow::ToggleSimSidebar(){
    show_app_sidebar = !show_app_sidebar;
    return show_app_sidebar;
}
bool ImguiWindow::ToggleDemoWindow(){
    show_demo_window = !show_demo_window;
    return show_demo_window;
}

void ImguiWindow::ShowWindowLicense() {
    float txtW = ImGui::CalcTextSize(" ").x;
    ImGui::SetNextWindowSize(ImVec2(txtW*120,0));
    ImGui::SetNextWindowPos(ImVec2(20, 20), ImGuiCond_FirstUseEver);
    if (ImGui::Begin("License", &show_window_license, ImGuiWindowFlags_AlwaysAutoResize)) {
        std::ostringstream aboutText;
        aboutText << "Program:    " << appName << " " << appVersionName << " (" << appVersionId << ")";
                    aboutText << R"(
Authors:     Roberto KERSEVAN / Marton ADY / Pascal BAEHR / Jean-Luc PONS
Copyright:   CERN / E.S.R.F.   (2024)
Website:    https://cern.ch/molflow

License summary: 
This program is free software, but we do not warrant that
it's free of bugs or that its results are valid.

Full license text: https://molflow.docs.cern.ch/about/

Open-source libraries used:
- SDL2 (https://www.libsdl.org/)
- Curl (https://curl.se/libcurl/)
- LibPng (http://www.libpng.org/pub/png/libpng.html)
- GCC (https://gcc.gnu.org/)
- 7-zip and lib7zip wrapper (https://7-zip.org/ and https://github.com/stonewell/lib7zip)
- NativeFileDialog extended (https://github.com/btzy/nativefiledialog-extended)
- GTK3 (on Linux. https://www.gtk.org/)
- pugixml (https://github.com/zeux/pugixml)
- cereal (https://github.com/USCiLab/cereal)
- fmt (https://github.com/fmtlib/fmt)
)";
        ImGui::TextWrapped(aboutText.str());
        ImGui::PlaceAtRegionCenter("  OK  ");
        if (ImGui::Button("  OK  ")) {
            show_window_license = false;
        }
        ImGui::End();
    }
}

// --- Toggle functions ---

// Setup Dear ImGui context and various default values (font, colors etc.)
void ImguiWindow::init() {
    if(didIinit) return;
    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
#ifdef DEBUG
    testEngine.Init(mApp);
#endif

    // io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable
    // Keyboard Controls io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //
    // Enable Gamepad Controls

    //---!!! must be after all other io.ConfigFlags changes !!!---
    ImguiWindow::storedConfigFlags = io.ConfigFlags; //save flags setup to allow restoring it (used to control mouse pointer drawing)

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f); //Frame borders
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f); //Window rounding

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
    int oversample = 1;
    static const ImWchar sym_ranges[] = {0x2000, 0x3000, 0};
    ImFontConfig fontConfig;
    fontConfig.MergeMode = true;
    fontConfig.PixelSnapH = true;
    fontConfig.OversampleH = oversample;
    fontConfig.OversampleV = oversample;
    //fontConfig.RasterizerMultiply = 0;
    //io.Fonts->AddFontDefault(&fontConfig);
    io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("fonst/FreeMono.ttf", 16.0f, &fontConfig, sym_ranges); // vector arrow

    // merge in icons from Font Awesome
    static const ImWchar icons_ranges[] = {ICON_MIN_FA, ICON_MAX_FA, 0};
    ImFontConfig icons_config;
    icons_config.MergeMode = true;
    icons_config.PixelSnapH = true;
    icons_config.OversampleH = oversample;
    icons_config.OversampleV = oversample;
    //icons_config.RasterizerMultiply = 0;
    icons_config.OversampleH = oversample;
    icons_config.OversampleV = oversample;
    //icons_config.RasterizerMultiply = 0;
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 16.0f, &icons_config, icons_ranges);

    io.Fonts->AddFontFromFileTTF("fonts/DroidSans.ttf", 14.0f);
    io.Fonts->AddFontFromFileTTF("fonts/FreeMono.ttf", 14.0f, &fontConfig, sym_ranges); // vector arrow
    io.Fonts->AddFontFromFileTTF(FONT_ICON_FILE_NAME_FAS, 14.0f, &icons_config, icons_ranges);

    io.Fonts->Build();

    /*io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 16.0f);
    io.Fonts->AddFontFromFileTTF("FreeMono.ttf", 16.0f, &fontConfig, sym_ranges);*/

// use FONT_ICON_FILE_NAME_FAR if you want regular instead of solid

    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    // io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    // ImFont* font =
    // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
    // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

    show_main_hub = false;
    show_demo_window = false;
    show_app_main_menu_bar = false;
    show_app_sidebar = false;
    show_perfo = false;
    show_window_license = false;

    popup = ImIOWrappers::ImPopup();
    input = ImIOWrappers::ImInputPopup();
    progress = ImProgress();
    progress.Init(mApp);
    smartSelect = ImSmartSelection();
    smartSelect.Init(mApp);
    selByNum = ImSelectDialog();
    selByNum.Init(mApp);
    selByTex = ImSelectTextureType();
    selByTex.Init(mApp);
    facetMov = ImFacetMove();
    facetMov.Init(mApp, mApp->worker.GetGeometry());
    globalSet = ImGlobalSettings();
    globalSet.Init(mApp);
    selFacetByResult = ImSelectFacetByResult();
    selFacetByResult.Init(mApp);

    shortcutMan = ShortcutManager();
    sideBar = ImGuiSidebar();
    formulaEdit = ImFormulaEditor();
    formulaEdit.Init(mApp, mApp->appFormulas);
    convPlot = ImConvergencePlotter();
    convPlot.Init(mApp);
    textPlot = ImTexturePlotter();
    textPlot.Init(mApp);
    profPlot = ImProfilePlotter();
    profPlot.Init(mApp);
    histPlot = ImHistogramPlotter();
    histPlot.Init(mApp);
    textScale = ImTextureScaling();
    textScale.Init(mApp);

    expFac = ImExplodeFacet();
    expFac.Init(mApp);

    RegisterShortcuts();

    start_time = ImGui::GetTime();
    didIinit = true;
    renderSingle();
}

// Gracefully clears and shutsdown Dear ImGui context
void ImguiWindow::destruct() {
    // Cleanup
    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplSDL2_Shutdown();
#ifdef DEBUG
    testEngine.StopEngine();
#endif
    ImPlot::DestroyContext();
    ImGui::DestroyContext();
#ifdef DEBUG
    testEngine.DestroyContext();
#endif
}

// TODO: When Imgui will be the main window/main GUI, use a full render cycle instead of a single frame rendering call
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

// Function for an individual frame rendering step
// If active, renders the individual components and handles the corresponding user actions
void ImguiWindow::renderSingle() {
    this->init();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

#if defined(MOLFLOW)
    MolFlow *mApp = (MolFlow *) app;
#else
    SynRad *mApp = (SynRad *) app;
#endif
    io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
    if (mApp) {
        bool redrawAabb = false;
        bool rebuildAabb = false;
        static int nbProc = static_cast<int>(mApp->worker.GetProcNumber());


        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(/*app->mainScreen*/);
        ImGui::NewFrame();

        if (show_app_main_menu_bar)
            ShowAppMainMenuBar();

        if (show_app_sidebar)
            sideBar.ShowAppSidebar(&show_app_sidebar, mApp, mApp->worker.GetGeometry());

        // 1. Show the big demo window (Most of the sample code is in
        // ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear
        // ImGui!).
        if ((io.KeyCtrl && io.KeyShift && io.KeyAlt && ImGui::IsKeyDown(ImGuiKey_D)))
            show_demo_window = !show_demo_window;
        if (show_demo_window) {
            ImGui::ShowDemoWindow(&show_demo_window);
            ImPlot::ShowDemoWindow(&show_demo_window);
        }

        // 2. Show Molflow x ImGui Hub window
        if (show_main_hub) {
            ImGui::SetNextWindowPos(ImVec2(20,20), ImGuiCond_FirstUseEver);
            ImGui::Begin("[BETA] _Molflow ImGui Suite_", &show_main_hub, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings); // Create a window called "Hello, world!"
            // and append into it.

            
            ImGui::Checkbox(
                    "Demo Window",
                    &show_demo_window); // Edit bools storing our window open/close state
            ImGui::Checkbox("Sidebar [NOT WORKING]", &show_app_sidebar);
            
            bool globalSettings = globalSet.IsVisible();
            if (ImGui::Checkbox("Global settings", &globalSettings)) {
                globalSet.SetVisible(globalSettings);
            }
            ImGui::Checkbox("Menu bar", &show_app_main_menu_bar);
            ImGui::Checkbox("Performance Plot", &show_perfo);
            ImGui::Checkbox("Demo window",&show_demo_window);
#ifdef DEBUG
            bool testEngineVis = testEngine.IsVisible();
            if (ImGui::Checkbox("Test Engine", &testEngineVis)) {
                testEngine.SetVisible(testEngineVis);
            }
#endif

            static int response;
            if (ImGui::CollapsingHeader("Popups")) {
                ImGui::BeginChild("Popup", ImVec2(0.f, ImGui::GetTextLineHeightWithSpacing() * 3));
                if (ImGui::Button("Test Popup Wrapper")) {
                    popup.Open("Title##0", "Message", { 
                        std::make_shared<ImIOWrappers::ImButtonInt>("OK", ImIOWrappers::buttonOk, ImGuiKey_Enter),
                        std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImIOWrappers::buttonCancel, ImGuiKey_Escape)
                        }); // Open wrapped popup
                }
                if (popup.WasResponse()) { // if there was a response
                    response = popup.GetResponse(); // do something
                }
                ImGui::Text("Popup response: "+std::to_string(response));
                ImGui::EndChild();
                static float prog;
                if (ImGui::SliderFloat("Progress", &prog, 0, 1))
                    progress.SetProgress(prog);
                if (ImGui::Button("Toggle progress bar")) {
                    progress.SetMessage("Message");
                    progress.SetTitle("Title##1");
                    progress.Toggle();
                }
            }
            if (ImGui::CollapsingHeader("Shortcuts")) {
                ImGui::Text("Register ctrl+shift+t");
                if (ImGui::Button("Register")) {
                    std::function<void()> F = []() { ImIOWrappers::InfoPopup("Shortcut", "I was opened by a keyboard shortcut"); };
                    shortcutMan.RegisterShortcut({ SDL_SCANCODE_LCTRL,SDL_SCANCODE_LSHIFT,SDL_SCANCODE_T }, F, 100);
                }
                ImGui::Text("Unegister ctrl+shift+t");
                if (ImGui::Button("Unregister")) {
                    shortcutMan.UnregisterShortcut(100);
                }
            }
            

            ImGui::Text("Avg %.3f ms/frame (%.1f FPS)",
                        1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            auto now_time = ImGui::GetTime();
            ImGui::Text("Application time %.3f s [%.3f s]",
                        ImGui::GetTime(), difftime(static_cast<time_t>(now_time), static_cast<time_t>(start_time)));
            ImGui::End();
        }
#ifdef DEBUG
        testEngine.Draw();
#endif
        // 3. Show window plotting the simulation performance
        if (show_perfo) {
            ShowPerfoPlot(&show_perfo, mApp);
        }

        if (show_window_license)
            ShowWindowLicense();

        // reusable windows for I/O
        popup.Draw();
        input.Draw();
        progress.Draw();

        smartSelect.Draw();
        selByNum.Draw();
        selByTex.Draw();
        facetMov.Draw();
        globalSet.Draw();
        selFacetByResult.Draw();
        formulaEdit.Draw();
        convPlot.Draw();
        textPlot.Draw();
        profPlot.Draw();
        histPlot.Draw();
        textScale.Draw();

        expFac.Draw();

        shortcutMan.DoShortcuts();

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        // SDL_GL_SwapWindow(app->mainScreen);

        // This allows for ImGui to render its cursor only if an ImGui element is focused, otherwise it allows the default cursor
        // Produces unpredictable behaviour when changing focus between ImGui and Legacy interfaces
        // Has issiues related to ImGui rendering being paused after inactivity
        /*if (ImGui::IsAnyItemHovered())
        {
            io.ConfigFlags = ImguiWindow::storedConfigFlags;
        }
        else
        {
            io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        }*/
    }
}

void ImguiWindow::Refresh()
{
    histPlot.RefreshFacetLists();
    histPlot.LoadHistogramSettings();
    convPlot.Reload();
}

void ImguiWindow::Reset()
{
    histPlot.Reset();
    histPlot.RefreshFacetLists();
    histPlot.LoadHistogramSettings();
}

void ImguiWindow::LoadProfileFromFile(const std::unique_ptr<MolflowInterfaceSettings>& interfaceSettings)
{
    if (interfaceSettings->profilePlotterSettings.hasData) {
        profPlot.LoadSettingsFromFile(interfaceSettings->profilePlotterSettings.logYscale, interfaceSettings->profilePlotterSettings.viewIds);
    }
    if (interfaceSettings->convergencePlotterSettings.hasData) {
        convPlot.LoadSettingsFromFile(interfaceSettings->convergencePlotterSettings.logYscale, interfaceSettings->convergencePlotterSettings.viewIds);
    } else convPlot.Reload();
    textScale.Load();
}
