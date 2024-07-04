

#include "ImguiGlobalSettings.h"
#include "ImguiExtensions.h"
#include "imgui.h"
#include <imgui_internal.h>
#include <sstream>
#include "Helper/StringHelper.h"
#include "ImguiWindow.h"
#include "ImguiPopup.h"

#include "Interface/AppUpdater.h"
#include "../../src/Interface/GlobalSettings.h"

void ImGlobalSettings::ProcessControlTable() {
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


    ImVec2 outer_size = ImVec2(0.0f, ImGui::GetFrameHeight() - (TEXT_BASE_HEIGHT * 3.5f));
    if (ImGui::BeginTable("procTable", 5, flags, outer_size)) {

        ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
        ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, TEXT_BASE_WIDTH * 10.f);
        ImGui::TableSetupColumn("PID", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Mem Usage", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Mem Peak", ImGuiTableColumnFlags_WidthFixed);
        ImGui::TableSetupColumn("Status", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableHeadersRow();

        int time = SDL_GetTicks();
        if (time - lastUpdate >= 500) {
            mApp->worker.GetProcStatus(procInfo);


            byte_to_mbyte = 1.0 / (1024.0 * 1024.0);
            //Interface
#ifdef _WIN32
            currPid = GetCurrentProcessId();
            memDenominator_sys = (1024.0 * 1024.0);
#else
            size_t currPid = getpid();
            memDenominator_sys = (1024.0);
#endif
            PROCESS_INFO parentInfo{};
            GetProcInfo(static_cast<DWORD>(currPid), &parentInfo);
            lastUpdate = SDL_GetTicks();
        }
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("Interface");
        ImGui::TableSetColumnIndex(1);
        ImGui::Text("%zd", currPid);
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.0f MB", (double) parentInfo.mem_use / memDenominator_sys);
        ImGui::TableSetColumnIndex(3);
        ImGui::Text("%.0f MB", (double) parentInfo.mem_peak / memDenominator_sys);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text("[Geom: %s]", mApp->worker.model->sh.name.c_str());

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::Text("SimManager");
        ImGui::TableSetColumnIndex(2);
        ImGui::Text("%.0f MB", (double)mApp->worker.model->memSizeCache * byte_to_mbyte);
        ImGui::TableSetColumnIndex(4);
        ImGui::Text(mApp->worker.GetSimManagerStatus().c_str());

// Demonstrate using clipper for large vertical lists
        ImGuiListClipper clipper;
        clipper.Begin(static_cast<int>(procInfo.threadInfos.size()));
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                size_t i = row + 2;
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("Thread %zu", i - 1);

                auto& proc = procInfo.threadInfos[i - 2];

                ImGui::TableSetColumnIndex(2);
                ImGui::Text("%.0f MB", (double)proc.runtimeInfo.counterSize * byte_to_mbyte);

                ImGui::TableSetColumnIndex(4);
                ImGui::Text("[%s] %s", threadStateStrings[proc.threadState].c_str(), proc.threadStatus.c_str());
            }
        }
        ImGui::EndTable();
    }
}


void ImGlobalSettings::Draw() {
    if (!drawn) return;
    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowMinSize,
            ImVec2(85 * txtW, 30 * txtH )); // Lift normal size constraint, however the presence of
    // a menu-bar will give us the minimum height we want.
    ImGui::SetNextWindowPos(ImVec2(20.f, 20.f), ImGuiCond_FirstUseEver);
    ImGui::Begin(
        "Global settings", &drawn,
        ImGuiWindowFlags_NoSavedSettings);  // Pass a pointer to our bool
    // variable (the window will have
    // a closing button that will
    // clear the bool when clicked)
    ImGui::PopStyleVar(1);

    float gasMass = 2.4f;
    bool appSettingsChanged = false; //To sync old global settings window
    if (ImGui::BeginTable("split", 2, ImGuiTableFlags_BordersInnerV)) {
        ImGui::TableSetupColumn("App settings (applied immediately)");
        ImGui::TableSetupColumn("Simulation settings (current file)");
        ImGui::TableHeadersRow();
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::PushItemWidth(ImGui::CalcTextSize("0").y * 3);
        appSettingsChanged |= ImGui::InputDouble("Autosave frequency (minutes)",
            &mApp->autoSaveFrequency, 0.00f, 0.0f, "%.1f");
        ImGui::PopItemWidth();
        appSettingsChanged |= ImGui::Checkbox(
            "Autosave only when simulation is running",
            reinterpret_cast<bool*>(
                &mApp->autoSaveSimuOnly)); // Edit bools storing our window
        // open/close state
        appSettingsChanged |= ImGui::Checkbox(
            "Use .zip as default extension (otherwise .xml)",
            reinterpret_cast<bool*>(
                &mApp->compressSavedFiles)); // Edit bools storing our window
        // open/close state
        if (!mApp->appUpdater) ImGui::BeginDisabled();
        static bool updateCheckPreference;
        if (mApp->appUpdater) { //Updater initialized
            updateCheckPreference = mApp->appUpdater->IsUpdateCheckAllowed();
        }
        else {
            updateCheckPreference = false;
        }
        if (appSettingsChanged |= ImGui::Checkbox(
            "Check for updates at startup",
            &updateCheckPreference)) {
            mApp->appUpdater->SetUserUpdatePreference(updateCheckPreference);
        };
        if (!mApp->appUpdater) ImGui::EndDisabled();

        // open/close state
        appSettingsChanged |= ImGui::Checkbox("Anti-Aliasing",
            &mApp->antiAliasing); // Edit bools storing our window
        // open/close state
        appSettingsChanged |= ImGui::Checkbox(
            "White Background",
            &mApp->whiteBg); // Edit bools storing our window open/close state
        if (ImGui::Checkbox("Left-handed coord. system", &mApp->leftHandedView)) {
            appSettingsChanged |= true;
            for (auto& i : mApp->viewers) {
                i->UpdateMatrix();
                i->UpdateLabelColors();
            }
        }
        ImGui::Checkbox(
            "Highlight non-planar facets",
            &mApp->highlightNonplanarFacets); // Edit bools storing our window
        // open/close state
        appSettingsChanged |= ImGui::Checkbox("Highlight selected facets",
            &mApp->highlightSelection); // Edit bools storing our
        // window open/close state
        appSettingsChanged |= ImGui::Checkbox("Use old XML format",
            &mApp->useOldXMLFormat); // Edit bools storing our
        // window open/close state
        ImGui::TableNextColumn();
        ImGui::PushItemWidth(100);

        /* --- Simu settings ---*/
#if defined(MOLFLOW)
        static double gasMass = mApp->worker.model->sp.gasMass;
        static bool enableDecay = mApp->worker.model->sp.enableDecay;
        static double halfLife = mApp->worker.model->sp.halfLife;
        static bool lowFluxMode = mApp->worker.model->otfParams.lowFluxMode;
        static double lowFluxCutoff = mApp->worker.model->otfParams.lowFluxCutoff;
        ImGui::InputDoubleRightSide("Gas molecular mass (g/mol)", &gasMass, "%g", "###GMass");
        ImGui::Checkbox("##EnableDecay", &enableDecay);
        if (!enableDecay) {
            ImGui::BeginDisabled();
        }
        ImGui::SameLine();
        ImGui::InputDoubleRightSide("Gas half life (s)", &halfLife, "%g", "###Gas half life (s)");

        if (!enableDecay) {
            ImGui::EndDisabled();
        }

        ImGui::Checkbox(
            "Enable low flux mode",
            &lowFluxMode);
        ImGui::SameLine();
        ImGui::HelpMarker(
            "Low flux mode helps to gain more statistics on low pressure "
            "parts of the system, at the expense"
            "of higher pressure parts. If a traced particle reflects from a "
            "high sticking factor surface, regardless of that probability,"
            "a reflected test particle representing a reduced flux will "
            "still be traced. Therefore test particles can reach low flux "
            "areas more easily, but"
            "at the same time tracing a test particle takes longer. The "
            "cutoff ratio defines what ratio of the originally generated "
            "flux"
            "can be neglected. If, for example, it is 0.001, then, when "
            "after subsequent reflections the test particle carries less "
            "than 0.1%"
            "of the original flux, it will be eliminated. A good advice is "
            "that if you'd like to see pressure across N orders of "
            "magnitude, set it to 1E-N");
        if (!lowFluxMode) {
            ImGui::BeginDisabled();
        }
        ImGui::InputDoubleRightSide(
            "Cutoff ratio", &lowFluxCutoff,
            "%.2e", "###Cutoff ratio");
        if (!lowFluxMode) {
            ImGui::EndDisabled();
        }

        {
            bool settingsChanged = false, simChanged = false, changedMass = false;
            simChanged |= mApp->worker.model->sp.gasMass != gasMass;
            changedMass = simChanged;
            simChanged |= mApp->worker.model->sp.halfLife != halfLife;
            simChanged |= mApp->worker.model->sp.enableDecay != enableDecay;

            settingsChanged |= simChanged;

            settingsChanged |= mApp->worker.model->otfParams.lowFluxMode != lowFluxMode;
            settingsChanged |= mApp->worker.model->otfParams.lowFluxCutoff != lowFluxCutoff;
            bool wasDisabled = settingsChanged;
            ImGui::PlaceAtRegionCenter("Apply above settings");
            if (!settingsChanged) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Apply above settings")) {
                
                mApp->worker.model->otfParams.lowFluxMode = lowFluxMode;
                mApp->worker.model->otfParams.lowFluxCutoff = lowFluxCutoff;
                
                if (simChanged) {
                    LockWrapper myLock(mApp->imguiRenderLock);
                    if (mApp->AskToReset()) {
                        mApp->worker.model->sp.gasMass = gasMass;
                        mApp->worker.model->sp.halfLife = halfLife;
                        mApp->worker.model->sp.enableDecay = enableDecay;
                        mApp->worker.MarkToReload();
                        mApp->changedSinceSave = true;
                        mApp->UpdateFacetlistSelected();
                        mApp->UpdateViewers();
                        if (changedMass) {
                            ImIOWrappers::InfoPopup("You have changed the gas mass.", "Don't forget the pumps: update pumping speeds and/or recalculate sticking factors.");
                        }
                    }
                }

            }
            if (!settingsChanged) {
                ImGui::EndDisabled();
            }
        }
        ImGui::NewLine();
        {
            //ImGui::BeginDisabled();
            static char inputText[128] = "Model changed"; //Imgui only accepting C-style arrays // NOT ANYMORE, CAN UPGRAGE TO std::string
            if (mApp->worker.needsReload) sprintf(inputText, "Model changed");
            // TODO Display Model changed when there are changes to facets, settings etc. show values when sim started or outgassing recalculated
            if (!mApp->worker.needsReload) sprintf(inputText, "%g", mApp->worker.model->sp.finalOutgassingRate_Pa_m3_sec * PAM3S_TO_MBARLS);
            ImGui::InputTextRightSide("Final outgassing rate (mbar*l/sec)", inputText, ImGuiInputTextFlags_ReadOnly);
            if (!mApp->worker.needsReload) sprintf(inputText, "%g", mApp->worker.model->sp.finalOutgassingRate);
            ImGui::InputTextRightSide("Final outgassing rate (1/sec)", inputText, ImGuiInputTextFlags_ReadOnly); // In molecules/sec
            {
                char tmpLabel[64];
                sprintf(tmpLabel, "Tot.des. molecules [0 to %g s]", mApp->worker.model->sp.latestMoment);
                if (!mApp->worker.needsReload) sprintf(inputText, "%.3E", mApp->worker.model->sp.totalDesorbedMolecules);
                ImGui::InputTextRightSide(tmpLabel, inputText, ImGuiInputTextFlags_ReadOnly);
            }

            //ImGui::EndDisabled();
            //***********************

            {
                const std::string btnText = "Recalc. outgassing";
                ImGui::PlaceAtRegionRight(btnText.c_str(), false);
            }

            if (ImGui::Button("Recalc. outgassing")) {
                RecalculateOutgassing();
            }
        }

        if (appSettingsChanged) {
            if (mApp->globalSettings) {
                mApp->globalSettings->Update(); //backwards compatibility: sync old Global Settings window
            }
        }

#else
        //SYNRAD
#endif
        ImGui::PopItemWidth();
        ImGui::EndTable();
    }

    ProcessControlTable();

    ImGui::Text("Number of CPU cores:     %zd", mApp->numCPU);
    ImGui::Text("Number of subprocesses:  ");
    ImGui::SameLine();
    if(updateNbProc)
        nbProc = (static_cast<int>(mApp->worker.GetProcNumber()));
        updateNbProc = false;
    ImGui::SetNextItemWidth(ImGui::CalcTextSize("0").x * 10);
    ImGui::InputInt("##nbProc", &nbProc, 1, 8);

    if (nbProc > MAX_PROCESS) {
        nbProc = MAX_PROCESS;
    }
    if (nbProc < 1) {
        nbProc = 1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Apply and restart processes")) {
        RestartProc();
        updateNbProc = true;
    }
    {
        size_t maxDes = mApp->worker.model->otfParams.desorptionLimit;
        std::string label = ("Desorption limit:" + ((maxDes == 0) ? "Infinite" : fmt::format("{:.2e}", static_cast<float>(maxDes))));
        ImGui::PlaceAtRegionRight(label.c_str(), true);
        if (ImGui::Button(label.c_str())) {
            auto Func = [this](std::string arg) {
                size_t inputD = 0;
                if (!Util::getNumber(&inputD, arg)) {
                    ImIOWrappers::InfoPopup("Error", "Invalid input");
                    return;
                }
                mApp->worker.model->otfParams.desorptionLimit = inputD;
                };
            mApp->imWnd->input.Open("Change desorption limit", "Desorption max (0 for no limit)", Func, fmt::format("{:.2e}", static_cast<float>(maxDes)));
        }
    }
    ImGui::End();
}

void ImGlobalSettings::Init(Interface* mApp_)
{
    mApp = mApp_;
}

/**
 * \brief Function to apply changes to the number of processes.
 */
void ImGlobalSettings::RestartProc() {
    try {
        LockWrapper myLock(mApp->imguiRenderLock);
        mApp->worker.Stop_Public();
        mApp->worker.SetProcNumber(nbProc);
        mApp->worker.RealReload(true);
        mApp->SaveConfig();
    }
    catch (Error& e) {
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

void ImGlobalSettings::RecalculateOutgassing() {
    LockWrapper myLock(mApp->imguiRenderLock);
    if (mApp->AskToReset()) {
        try {
            mApp->worker.RealReload();
        }
        catch (std::exception& e) {
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
}