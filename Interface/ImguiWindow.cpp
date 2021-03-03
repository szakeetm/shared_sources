//
// Created by pascal on 2/26/21.
//

#include "ImguiWindow.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"
#include "../../src/MolFlow.h"

#include <sstream>
#include <imgui/imgui_internal.h>

void ImguiWindow::init() {
// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    //ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();
    ImGui::StyleColorsLight();

    // Setup Platform/Renderer backends
    ImGui_ImplSDL2_InitForOpenGL(app->mainScreen, app->mainContext);
    ImGui_ImplOpenGL2_Init();

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

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

void ImguiWindow::render() {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;

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
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE &&
                event.window.windowID == SDL_GetWindowID(app->mainScreen))
                done = true;
        }

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(app->mainScreen);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin(
                    "Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text(
                    "This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport((int) io.DisplaySize.x * 0.25, (int) io.DisplaySize.y * 0.25, (int) io.DisplaySize.x / 2.0,
                   (int) io.DisplaySize.y / 2.0);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(app->mainScreen);
    }
}

// Make the UI compact because there are so many fields
static void PushStyleCompact() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding,
                        ImVec2(style.FramePadding.x, (float) (int) (style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,
                        ImVec2(style.ItemSpacing.x, (float) (int) (style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact() {
    ImGui::PopStyleVar(2);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
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

static void InputRightSide(const char *desc, double *val, const char *format) {
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
}

// Add spacing of checkbox width
static void AddCheckboxWidthSpacing(){
    ImGui::NewLine();
    ImGui::SameLine(ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().FramePadding.y * 4);
}

void ImguiWindow::ProcessControllTable(MolFlow *mApp) {
    ImGui::Text("Process control");
    static ImGuiTableFlags flags =
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
            ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
    const float TEXT_BASE_WIDTH = ImGui::CalcTextSize("000000").x;
    if (ImGui::BeginTable("procTable", 5, flags)) {
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
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

        //Interface
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
        size_t currPid = GetCurrentProcessId();
        PROCESS_INFO parentInfo;
        GetProcInfo(currPid, &parentInfo);

        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Interface");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%zd", currPid);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.0f MB", (double) parentInfo.mem_use / (1024.0 * 1024.0));
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.0f MB", (double) parentInfo.mem_peak / (1024.0 * 1024.0));
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("");
#else
        size_t currPid = getpid();
                    PROCESS_INFO parentInfo;
                    GetProcInfo(currPid, &parentInfo);
                    ImGui::TableSetColumnIndex(0);
                    ImGui::Text("Interface");
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("%zd", currPid);
                    ImGui::TableSetColumnIndex(2);
                    ImGui::Text("%.0f MB", (double)parentInfo.mem_use / (1024.0));
                    ImGui::TableSetColumnIndex(3);
                    ImGui::Text("%.0f MB", (double)parentInfo.mem_peak / (1024.0));
                    ImGui::TableSetColumnIndex(4);
                    ImGui::Text("");
#endif
        size_t i = 1;
        for (auto &proc : procInfo.subProcInfo) {
            //auto& proc = procInfo.subProcInfo[0];
            DWORD pid = proc.procId;
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::Text("Subproc.%zu", i);
            ImGui::TableSetColumnIndex(1);
            ImGui::Text("%lu", pid);
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
            PROCESS_INFO pInfo = proc.runtimeInfo;
            {

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.0f MB", (double) pInfo.mem_use / (1024.0 * 1024.0));
                ImGui::TableSetColumnIndex(3);
                ImGui::Text("%.0f MB", (double) pInfo.mem_peak / (1024.0 * 1024.0));
                // State/Status
                std::stringstream tmp_ss; //tmp_ss << "[" << prStates[states[i-1]] << "] " << statusStrings[i-1];
                tmp_ss << "[" << prStates[states[i - 1]] << "] " << statusStrings[i - 1];
                ImGui::TableSetColumnIndex(4);
                ImGui::Text("%s", tmp_ss.str().c_str());
            }

#else
            if (pid == currPid) { // TODO: Check if this is wanted
                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("0 KB");
                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("0 KB");
                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("Dead");
                        }
                        else {
                            PROCESS_INFO pInfo = proc.runtimeInfo;
                            //GetProcInfo(pid, &pInfo);

                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.0f MB", (double)pInfo.mem_use / (1024.0));
                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.0f MB", (double)pInfo.mem_peak / (1024.0));


                            // State/Status

                            std::stringstream tmp_ss; //tmp_ss << "[" << prStates[states[i-1]] << "] " << statusStrings[i-1];
                            tmp_ss << "[" << prStates[proc.slaveState] << "] " << proc.statusString;

                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("%s", tmp_ss.str().c_str());
                        }
#endif
            ++i;
        }
        ImGui::EndTable();
    }
}

void ImguiWindow::renderSingle() {
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Main loop
    bool done = false;
    //while (!done)
    MolFlow *mApp = (MolFlow *) app;
    if (mApp) {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        /*SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(app->mainScreen))
                done = true;
        }*/

        bool nbProcChanged = false;
        bool recalcOutg = false;
        bool changeDesLimit = false;
        static int nbProc = mApp->worker.GetProcNumber();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplSDL2_NewFrame(app->mainScreen);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin(
                    "Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text(
                    "This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float *) &clear_color); // Edit 3 floats representing a color

            if (ImGui::Button(
                    "Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate,
                        ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window) {
            if (mApp) {

                ImGui::Begin("Global settings",
                             &show_another_window,
                             ImGuiWindowFlags_NoSavedSettings);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                float gasMass = 2.4;
                if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersInnerV)) {
                    //PushStyleCompact();
                    //ImGui::TableNextColumn();
                    //ImGui::PushID(0);
                    //ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation
                    ImGui::TableSetupColumn("Global settings");
                    ImGui::TableSetupColumn("Simulation settings");
                    ImGui::TableHeadersRow();
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    ImGui::PushItemWidth(100);
                    ImGui::InputDouble("Autosave frequency (minutes)", &mApp->autoSaveFrequency, 0.00f, 0.0f, "%.1f");
                    ImGui::PopItemWidth();
                    ImGui::Checkbox("Autosave only when simulation is running",
                                    reinterpret_cast<bool *>(&mApp->autoSaveSimuOnly));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Use .zip as default extension (otherwise .xml)",
                                    reinterpret_cast<bool *>(&mApp->compressSavedFiles));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Check for updates at startup",
                                    reinterpret_cast<bool *>(&mApp->updateRequested));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Auto refresh formulas",
                                    reinterpret_cast<bool *>(&mApp->autoUpdateFormulas));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Anti-Aliasing",
                                    &mApp->antiAliasing);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("White Background",
                                    &mApp->whiteBg);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Left-handed coord. system",
                                    &mApp->leftHandedView);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Highlight non-planar facets",
                                    &mApp->highlightNonplanarFacets);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Highlight selected facets",
                                    &mApp->highlightSelection);      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Use old XML format",
                                    &mApp->useOldXMLFormat);      // Edit bools storing our window open/close state
                    ImGui::TableNextColumn();
                    //ImGui::PushID(1);
                    //ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation
                    //ImGui::TableSetupColumn("Simulation settings");
                    //ImGui::TableHeadersRow();
                    ImGui::PushItemWidth(100);



                    /* --- Simu settings ---*/
                    InputRightSide("Gas molecular mass (g/mol)", &mApp->worker.model.wp.gasMass, "%g");
                    ImGui::AlignTextToFramePadding();

                    ImGui::Checkbox("", &mApp->worker.model.wp.enableDecay);
                    if (!mApp->worker.model.wp.enableDecay) {
                        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(217, 217, 217, 255));
                    }
                    ImGui::SameLine();
                    InputRightSide("Gas half life (s)", &mApp->worker.model.wp.halfLife, "%g");

                    if (!mApp->worker.model.wp.enableDecay) {
                        ImGui::PopStyleColor();
                        ImGui::PopItemFlag();
                    }

                    ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                    ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(217, 217, 217, 255));

                    // Use tmp var to multiply by 10
                    double outgRate10 = mApp->worker.model.wp.finalOutgassingRate_Pa_m3_sec * 10.00;
                    InputRightSide("Final outgassing rate (mbar*l/sec)", &outgRate10,
                                   "%.4g"); //10: conversion Pa*m3/sec -> mbar*l/s
                    InputRightSide("Final outgassing rate (1/sec)",
                                   &mApp->worker.model.wp.finalOutgassingRate, "%.4g"); //In molecules/sec

                    {
                        char tmp[64];
                        sprintf(tmp, "Tot.des. molecules [0 to %g s]", mApp->worker.model.wp.latestMoment);
                        InputRightSide(tmp, &mApp->worker.model.wp.totalDesorbedMolecules, "%.4g");
                    }

                    ImGui::PopStyleColor();
                    ImGui::PopItemFlag();
                    //***********************

                    {
                        const std::string btnText = "Recalc. outgassing";
                        float font_size = ImGui::GetFontSize() * btnText.size() / 2;
                        ImGui::NewLine();
                        ImGui::SameLine((ImGui::GetContentRegionAvailWidth() / 2) -
                                        font_size + (font_size / 2));
                    }

                    if (ImGui::Button("Recalc. outgassing"))      // Edit bools storing our window open/close state
                        recalcOutg = true;
                    ImGui::Checkbox("Enable low flux mode",
                                    &mApp->worker.model.otfParams.lowFluxMode);      // Edit bools storing our window open/close state
                    ImGui::SameLine();
                    HelpMarker(
                            "Low flux mode helps to gain more statistics on low pressure parts of the system, at the expense\n"
                            "of higher pressure parts. If a traced particle reflects from a high sticking factor surface, regardless of that probability,\n"
                            "a reflected test particle representing a reduced flux will still be traced. Therefore test particles can reach low flux areas more easily, but\n"
                            "at the same time tracing a test particle takes longer. The cutoff ratio defines what ratio of the originally generated flux\n"
                            "can be neglected. If, for example, it is 0.001, then, when after subsequent reflections the test particle carries less than 0.1%\n"
                            "of the original flux, it will be eliminated. A good advice is that if you'd like to see pressure across N orders of magnitude, set it to 1E-N");
                    if (!mApp->worker.model.otfParams.lowFluxMode) {
                        ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
                        ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(217, 217, 217, 255));
                    }
                    ImGui::InputDouble("Cutoff ratio",
                                       &mApp->worker.model.otfParams.lowFluxCutoff, 0.0f, 0.0f,
                                       "%.2e");      // Edit bools storing our window open/close state
                    if (!mApp->worker.model.otfParams.lowFluxMode) {
                        ImGui::PopStyleColor();
                        ImGui::PopItemFlag();
                    }

                    ImGui::PopItemWidth();
                    //PopStyleCompact();
                    ImGui::EndTable();
                }

                {
                    const std::string btnText = "Apply above settings";
                    float font_size = ImGui::GetFontSize() * btnText.size() / 2;
                    ImGui::NewLine();
                    ImGui::SameLine((ImGui::GetWindowSize().x / 2) -
                                    font_size + (font_size / 2));
                }
                if (ImGui::Button("Apply above settings"))
                    show_another_window = false;

                // TODO: Insert process control
                ProcessControllTable(mApp);

                ImGui::Text("Number of CPU cores:     %zd", mApp->numCPU);
                ImGui::Text("Number of subprocesses:  ");
                ImGui::SameLine();
                /*bool nbProcChanged = false;
                int nbProc = mApp->worker.GetProcNumber();*/
                ImGui::SetNextItemWidth(50.0f);
                ImGui::DragInt("", &nbProc, 1, 0, MAX_PROCESS, "%d", ImGuiSliderFlags_AlwaysClamp);

                ImGui::SameLine();
                if (ImGui::Button("Apply and restart processes"))
                    nbProcChanged = true;
                {
                    const std::string btnText2 = "Change MAX desorbed molecules";
                    float font_size = ImGui::CalcTextSize(btnText2.c_str()).x;
                    ImGui::SameLine((ImGui::GetContentRegionAvailWidth()) - font_size);
                }
                if (ImGui::Button("Change MAX desorbed molecules"))
                    ImGui::OpenPopup("Edit MAX");

                // Always center this window when appearing
                ImVec2 center = ImGui::GetMainViewport()->GetCenter();
                ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

                if (ImGui::BeginPopupModal("Edit MAX", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                    //static bool initMax = false;
                    static double maxDes = mApp->worker.model.otfParams.desorptionLimit;;
                    /*if(!initMax) {
                        maxDes = mApp->worker.model.otfParams.desorptionLimit; // use double function to allow exponential format
                        initMax = true;
                    }*/
                    ImGui::InputDouble("Desorption max (0=>endless)", &maxDes, 0.0f, 0.0f, "%.0f");
                    ImGui::Separator();

                    //static int unused_i = 0;
                    //ImGui::Combo("Combo", &unused_i, "Delete\0Delete harder\0");

                    /*static bool dont_ask_me_next_time = false;
                    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
                    ImGui::Checkbox("Don't ask me next time", &dont_ask_me_next_time);
                    ImGui::PopStyleVar();*/

                    if (ImGui::Button("OK", ImVec2(120, 0))) {
                        mApp->worker.model.otfParams.desorptionLimit = maxDes;
                        //initMax = false;
                        changeDesLimit = true;
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::SetItemDefaultFocus();
                    ImGui::SameLine();
                    if (ImGui::Button("Cancel", ImVec2(120, 0))) { ImGui::CloseCurrentPopup(); }
                    ImGui::EndPopup();
                }

            }
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int) io.DisplaySize.x, (int) io.DisplaySize.y);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //SDL_GL_SwapWindow(app->mainScreen);


        // Handle button events at the end, because some functions call GL Repaint, which doesnt work well with ImGui if frame is not finalized
        if (nbProcChanged) {
            restartProc(nbProc, mApp);
        } else if (recalcOutg) {
            if (mApp->AskToReset()) {
                try {
                    mApp->worker.RealReload();
                }
                catch (std::exception &e) {
                    if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
                        ImGui::Text("Recalculation failed: Couldn't reload Worker:\n%s", e.what());
                        ImGui::Separator();

                        if (ImGui::Button("OK", ImVec2(120, 0))) {
                            ImGui::CloseCurrentPopup();
                        }
                        ImGui::SetItemDefaultFocus();
                        ImGui::EndPopup();
                    }
                }
            }
        }
        else if(changeDesLimit){
            mApp->worker.ChangeSimuParams(); //Sync with subprocesses
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
    }
    catch (Error &e) {
        if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
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