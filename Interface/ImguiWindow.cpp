//
// Created by pascal on 2/26/21.
//

#include "ImguiWindow.h"

#include "imgui/imgui.h"
#include "imgui/imgui_impl_sdl.h"
#include "imgui/imgui_impl_opengl2.h"
#include "../../src/MolFlow.h"

#include <sstream>
void ImguiWindow::init() {
// Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

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
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
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
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Our state
    bool show_demo_window = false;
    bool show_another_window = true;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSDL2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(app->mainScreen))
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

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            ImGui::Begin("Global settings", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            if (ImGui::BeginTable("split", 2))
            {
                ImGui::TableNextRow();
                ImGui::Checkbox("Autosave only when simulation is running", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Use .zip as default extension (otherwise .xml)", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Check for updates at startup", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Auto refresh formulas", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Anti-Aliasing", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("White Background", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Left-handed coord. system", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Highlight non-planar facets", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Highlight selected facets", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Use old XML format", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::TableNextRow();
                ImGui::Checkbox("Autosave only when simulation is running", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Use .zip as default extension (otherwise .xml)", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Check for updates at startup", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Auto refresh formulas", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Anti-Aliasing", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("White Background", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Left-handed coord. system", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Highlight non-planar facets", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Highlight selected facets", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::Checkbox("Use old XML format", &show_demo_window);      // Edit bools storing our window open/close state
                ImGui::EndTable();
            }

            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        glViewport((int)io.DisplaySize.x * 0.25, (int)io.DisplaySize.y * 0.25, (int)io.DisplaySize.x/2.0, (int)io.DisplaySize.y/2.0);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(app->mainScreen);
    }
}

// Make the UI compact because there are so many fields
static void PushStyleCompact()
{
    ImGuiStyle& style = ImGui::GetStyle();
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(style.FramePadding.x, (float)(int)(style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(style.ItemSpacing.x, (float)(int)(style.ItemSpacing.y * 0.60f)));
}

static void PopStyleCompact()
{
    ImGui::PopStyleVar(2);
}

static void EditTableColumnsFlags(ImGuiTableColumnFlags* p_flags)
{
    ImGui::CheckboxFlags("_DefaultHide", p_flags, ImGuiTableColumnFlags_DefaultHide);
    ImGui::CheckboxFlags("_DefaultSort", p_flags, ImGuiTableColumnFlags_DefaultSort);
    if (ImGui::CheckboxFlags("_WidthStretch", p_flags, ImGuiTableColumnFlags_WidthStretch))
        *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthStretch);
    if (ImGui::CheckboxFlags("_WidthFixed", p_flags, ImGuiTableColumnFlags_WidthFixed))
        *p_flags &= ~(ImGuiTableColumnFlags_WidthMask_ ^ ImGuiTableColumnFlags_WidthFixed);
    ImGui::CheckboxFlags("_NoResize", p_flags, ImGuiTableColumnFlags_NoResize);
    ImGui::CheckboxFlags("_NoReorder", p_flags, ImGuiTableColumnFlags_NoReorder);
    ImGui::CheckboxFlags("_NoHide", p_flags, ImGuiTableColumnFlags_NoHide);
    ImGui::CheckboxFlags("_NoClip", p_flags, ImGuiTableColumnFlags_NoClip);
    ImGui::CheckboxFlags("_NoSort", p_flags, ImGuiTableColumnFlags_NoSort);
    ImGui::CheckboxFlags("_NoSortAscending", p_flags, ImGuiTableColumnFlags_NoSortAscending);
    ImGui::CheckboxFlags("_NoSortDescending", p_flags, ImGuiTableColumnFlags_NoSortDescending);
    ImGui::CheckboxFlags("_NoHeaderWidth", p_flags, ImGuiTableColumnFlags_NoHeaderWidth);
    ImGui::CheckboxFlags("_PreferSortAscending", p_flags, ImGuiTableColumnFlags_PreferSortAscending);
    ImGui::CheckboxFlags("_PreferSortDescending", p_flags, ImGuiTableColumnFlags_PreferSortDescending);
    ImGui::CheckboxFlags("_IndentEnable", p_flags, ImGuiTableColumnFlags_IndentEnable); ImGui::SameLine();
    ImGui::CheckboxFlags("_IndentDisable", p_flags, ImGuiTableColumnFlags_IndentDisable); ImGui::SameLine();
}

static void ShowTableColumnsStatusFlags(ImGuiTableColumnFlags flags)
{
    ImGui::CheckboxFlags("_IsEnabled", &flags, ImGuiTableColumnFlags_IsEnabled);
    ImGui::CheckboxFlags("_IsVisible", &flags, ImGuiTableColumnFlags_IsVisible);
    ImGui::CheckboxFlags("_IsSorted", &flags, ImGuiTableColumnFlags_IsSorted);
    ImGui::CheckboxFlags("_IsHovered", &flags, ImGuiTableColumnFlags_IsHovered);
}

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a merged icon fonts (see docs/FONTS.md)
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ImguiWindow::renderSingle() {
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Main loop
    bool done = false;
    //while (!done)
    {
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

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);

            ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
            ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

            if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
                counter++;
            ImGui::SameLine();
            ImGui::Text("counter = %d", counter);

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
            ImGui::End();
        }

        // 3. Show another simple window.
        if (show_another_window)
        {
            MolFlow* mApp = (MolFlow*) app;
            if(mApp) {

                ImGui::Begin("Global settings",
                             &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
                float gasMass = 2.4;
                if (ImGui::BeginTable("split", 2)) {
                    PushStyleCompact();
                    ImGui::TableNextColumn();
                    ImGui::PushID(0);
                    //ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation
                    ImGui::Text("Global settings");
                    ImGui::Checkbox("Autosave only when simulation is running",
                                    reinterpret_cast<bool *>(&mApp->autoSaveSimuOnly));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Use .zip as default extension (otherwise .xml)",
                                    reinterpret_cast<bool *>(&mApp->compressSavedFiles));      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Check for updates at startup",
                                    reinterpret_cast<bool *>(&mApp->autoUpdateFormulas));      // Edit bools storing our window open/close state
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
                    ImGui::PushID(1);
                    //ImGui::AlignTextToFramePadding(); // FIXME-TABLE: Workaround for wrong text baseline propagation
                    ImGui::Text("Simulation settings");
                    ImGui::PushItemWidth(100);
                    ImGui::InputDouble("Gas molecular mass (g/mol) ##1b", &mApp->worker.model.wp.gasMass);
                    ImGui::InputDouble("Gas half life (s) ##1b", &mApp->worker.model.wp.halfLife);
                    ImGui::InputDouble("Final outgassing rate (mbar*l/sec) ##1b",
                                     &mApp->worker.model.wp.finalOutgassingRate);      // Edit bools storing our window open/close state
                    ImGui::InputDouble("Final outgassing rate (1/sec) ##1b",
                                     &mApp->worker.model.wp.finalOutgassingRate_Pa_m3_sec);      // Edit bools storing our window open/close state
                    ImGui::InputDouble("Total desorbed molecules:",
                                     &mApp->worker.model.wp.totalDesorbedMolecules);      // Edit bools storing our window open/close state
                    ImGui::Button("Recalc. outgassing");      // Edit bools storing our window open/close state
                    ImGui::Checkbox("Enable low flux mode",
                                    &mApp->worker.model.otfParams.lowFluxMode);      // Edit bools storing our window open/close state
                    ImGui::SameLine();
                    HelpMarker(
                            "Using TableSetupColumn() to alter resizing policy on a per-column basis.\n\n"
                            "When combining Fixed and Stretch columns, generally you only want one, maybe two trailing columns to use _WidthStretch.");
                    ImGui::InputDouble("Cutoff ratio ##1b",
                                     &mApp->worker.model.otfParams.lowFluxCutoff);      // Edit bools storing our window open/close state
                    ImGui::PopItemWidth();
                    PopStyleCompact();
                    ImGui::EndTable();
                }


                if (ImGui::Button("Apply above settings"))
                    show_another_window = false;

                ImGui::Text("Process control");
                static ImGuiTableFlags flags =
                        ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
                        ImGuiTableFlags_Resizable | ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable;
                if (ImGui::BeginTable("table2", 5, flags)) {
                    ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Mem Usage", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Mem Peak", ImGuiTableColumnFlags_WidthFixed);
                    ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);
                    ImGui::TableHeadersRow();

                    char tmp[512];
                    size_t  states[MAX_PROCESS];
                    std::vector<std::string> statusStrings(MAX_PROCESS);
                    memset(states, 0, MAX_PROCESS * sizeof(int));
                    mApp->worker.GetProcStatus(states, statusStrings);

                    ProcComm procInfo;
                    mApp->worker.GetProcStatus(procInfo);

                    for (int row = 0; row < 5; row++) {
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
    ImGui::Text("%.0f MB", (double)parentInfo.mem_use / (1024.0*1024.0));
    ImGui::TableSetColumnIndex(3);
    ImGui::Text("%.0f MB", (double)parentInfo.mem_peak / (1024.0*1024.0));
    ImGui::TableSetColumnIndex(4);
    ImGui::Text("");
    ImGui::TableSetColumnIndex(5);
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
                        ImGui::TableSetColumnIndex(5);
                        ImGui::Text("");
#endif
                        size_t i = 1;
                        for (auto& proc : procInfo.subProcInfo)
                        {
                            //auto& proc = procInfo.subProcInfo[0];
                            DWORD pid = proc.procId;
                            ImGui::TableSetColumnIndex(0);
                            ImGui::Text("Subproc.%lu", i);
                            ImGui::TableSetColumnIndex(1);
                            ImGui::Text("%d", pid);
                            ImGui::TableSetColumnIndex(2);
                            ImGui::Text("%.0f MB", (double)parentInfo.mem_use / (1024.0));
                            ImGui::TableSetColumnIndex(3);
                            ImGui::Text("%.0f MB", (double)parentInfo.mem_peak / (1024.0));
                            ImGui::TableSetColumnIndex(4);
                            ImGui::Text("");
                            ImGui::TableSetColumnIndex(5);
                            ImGui::Text("");
#if defined(WIN32) || defined(_WIN32) || defined(WIN64) || defined(_WIN64)
                            PROCESS_INFO pInfo = proc.runtimeInfo;
        /*if (!GetProcInfo(pid, &pInfo)) {
			processList->SetValueAt(2, i, "0 KB");
			processList->SetValueAt(3, i, "0 KB");
			//processList->SetValueAt(4,i,"0 %");
			processList->SetValueAt(4, i, "Dead");
		}
		else*/
        {
            sprintf(tmp, "%.0f MB", (double)pInfo.mem_use / (1024.0*1024.0));
            processList->SetValueAt(2, i, tmp);
            sprintf(tmp, "%.0f MB", (double)pInfo.mem_peak / (1024.0*1024.0));
            processList->SetValueAt(3, i, tmp);

			// State/Status
			std::stringstream tmp; tmp << "[" << prStates[states[i-1]] << "] " << statusStrings[i-1];
			processList->SetValueAt(4, i, tmp.str().c_str());
		}

#else
                            if (pid == currPid) { // TODO: Check if this is wanted
                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("0 KB");
                                ImGui::TableSetColumnIndex(3);
                                ImGui::Text("0 KB");
                                ImGui::TableSetColumnIndex(4);
                                ImGui::Text("Dead");
                                ImGui::TableSetColumnIndex(5);
                                ImGui::Text("");
                            }
                            else {
                                PROCESS_INFO pInfo = proc.runtimeInfo;
                                //GetProcInfo(pid, &pInfo);

                                ImGui::TableSetColumnIndex(2);
                                ImGui::Text("%.0f MB", (double)parentInfo.mem_use / (1024.0));
                                ImGui::TableSetColumnIndex(3);
                                ImGui::Text("%.0f MB", (double)parentInfo.mem_peak / (1024.0));


                                // State/Status

                                std::stringstream tmp_ss; //tmp_ss << "[" << prStates[states[i-1]] << "] " << statusStrings[i-1];
                                tmp_ss << "[" << prStates[proc.slaveState] << "] " << proc.statusString;

                                ImGui::TableSetColumnIndex(4);
                                ImGui::Text("%s", tmp_ss.str().c_str());
                                ImGui::TableSetColumnIndex(5);
                                ImGui::Text("");
                            }
#endif
                            ++i;
                        }
                        /*for (int column = 0; column < 6; column++) {
                            ImGui::TableSetColumnIndex(column);
                            ImGui::Text("%s %d", (column >= 4) ? "X Y Z" : "", row);
                        }*/
                    }
                    ImGui::EndTable();
                }
                ImGui::End();
            }
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        //glClearColor(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
        //glClear(GL_COLOR_BUFFER_BIT);
        //glUseProgram(0); // You may want this if using this code in an OpenGL 3+ context where shaders may be bound
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        //SDL_GL_SwapWindow(app->mainScreen);
    }
}
