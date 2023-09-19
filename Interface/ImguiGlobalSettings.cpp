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

#include "ImguiGlobalSettings.h"
#include "ImguiExtensions.h"
#include "imgui/imgui.h"
#include <imgui/imgui_internal.h>
#include <sstream>

#include "Interface/AppUpdater.h"
#include "../../src/Interface/GlobalSettings.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow *mApp;
#else
#include "../src/SynRad.h"
extern SynRad*mApp;
#endif

static void ProcessControlTable(Interface *mApp) {
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

        ProcComm procInfo;
        mApp->worker.GetProcStatus(procInfo);

        ImGui::TableNextRow();

        double byte_to_mbyte = 1.0 / (1024.0 * 1024.0);
        //Interface
#ifdef _WIN32
        size_t currPid = GetCurrentProcessId();
        double memDenominator_sys = (1024.0 * 1024.0);
#else
        size_t currPid = getpid();
        double memDenominator_sys = (1024.0);
#endif
        PROCESS_INFO parentInfo{};
        GetProcInfo(currPid, &parentInfo);
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
        clipper.Begin(procInfo.threadInfos.size());
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


void ShowGlobalSettings(Interface *mApp, bool *show_global_settings, int &nbProc) {
    int txtW = ImGui::CalcTextSize(" ").x;
    int txtH = ImGui::GetTextLineHeightWithSpacing();
    ImGui::PushStyleVar(
            ImGuiStyleVar_WindowMinSize,
            ImVec2(170 * txtW, 30 * txtH )); // Lift normal size constraint, however the presence of
    // a menu-bar will give us the minimum height we want.
    ImGui::Begin(
            "Global settings", show_global_settings,
            ImGuiWindowFlags_NoSavedSettings); // Pass a pointer to our bool
    // variable (the window will have
    // a closing button that will
    // clear the bool when clicked)
    ImGui::PopStyleVar(1);

    float gasMass = 2.4;
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
        appSettingsChanged |= ImGui::Checkbox(
            "Auto refresh formulas",
            reinterpret_cast<bool*>(
                &mApp->autoUpdateFormulas)); // Edit bools storing our window
        // open/close state
        appSettingsChanged |= ImGui::Checkbox("Anti-Aliasing",
            &mApp->antiAliasing); // Edit bools storing our window
        // open/close state
        appSettingsChanged |= ImGui::Checkbox(
            "White Background",
            &mApp->whiteBg); // Edit bools storing our window open/close state
        appSettingsChanged |= ImGui::Checkbox("Left-handed coord. system",
            &mApp->leftHandedView); // Edit bools storing our
        // window open/close state
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
        static bool simChanged = false;
#if defined(MOLFLOW)
        static double gasMass = mApp->worker.model->sp.gasMass;
        static bool enableDecay = mApp->worker.model->sp.enableDecay;
        static double halfLife = mApp->worker.model->sp.halfLife;
        static bool lowFluxMode = mApp->worker.model->otfParams.lowFluxMode;
        static double lowFluxCutoff = mApp->worker.model->otfParams.lowFluxCutoff;
        simChanged |= ImGui::InputDoubleRightSide("Gas molecular mass (g/mol)", &gasMass, "%g");
        simChanged |= ImGui::Checkbox("", &enableDecay);
        if (!enableDecay) {
            ImGui::BeginDisabled();
        }
        ImGui::SameLine();
        simChanged |= ImGui::InputDoubleRightSide("Gas half life (s)", &halfLife, "%g");

        if (!enableDecay) {
            ImGui::EndDisabled();
        }

        simChanged |= ImGui::Checkbox(
                "Enable low flux mode",
                &lowFluxMode);
        ImGui::SameLine();
        ImGui::HelpMarker(
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
            ImGui::BeginDisabled();
        }
        simChanged |= ImGui::InputDoubleRightSide(
                "Cutoff ratio", &lowFluxCutoff,
                "%.2e");
        if (!lowFluxMode) {
            ImGui::EndDisabled();
        }

        {
            bool wasDisabled = !simChanged;
            ImGui::PlaceAtRegionCenter("Apply above settings");
            if (wasDisabled) {
                ImGui::BeginDisabled();
            }
            if (ImGui::Button("Apply above settings")) {
                simChanged = false;
                mApp->worker.model->sp.gasMass = gasMass;
                mApp->worker.model->sp.enableDecay = enableDecay;
                mApp->worker.model->sp.halfLife = halfLife;
                mApp->worker.model->otfParams.lowFluxMode = lowFluxMode;
                mApp->worker.model->otfParams.lowFluxCutoff = lowFluxCutoff;
                
            }
            if (wasDisabled) {
                ImGui::EndDisabled();
            }
        }
        ImGui::NewLine();
        {
            ImGui::BeginDisabled();

            static char inputText[128] = "Model changed"; //Imgui only accepting C-style arrays

            if (!mApp->worker.needsReload) sprintf(inputText, "%g", mApp->worker.model->sp.finalOutgassingRate_Pa_m3_sec * PAM3S_TO_MBARLS);
            ImGui::InputTextRightSide("Final outgassing rate (mbar*l/sec)", inputText);
            if (!mApp->worker.needsReload) sprintf(inputText, "%g", mApp->worker.model->sp.finalOutgassingRate);
            ImGui::InputTextRightSide("Final outgassing rate (1/sec)", inputText); // In molecules/sec
            {
                char tmpLabel[64];
                sprintf(tmpLabel, "Tot.des. molecules [0 to %g s]", mApp->worker.model->sp.latestMoment);
                if (!mApp->worker.needsReload) sprintf(inputText, "%.3E", mApp->worker.model->sp.totalDesorbedMolecules);
                ImGui::InputTextRightSide(tmpLabel, inputText);
            }

            ImGui::EndDisabled();
            //***********************

            {
                const std::string btnText = "Recalc. outgassing";
                ImGui::PlaceAtRegionRight(btnText.c_str(), false);
            }

            if (ImGui::Button("Recalc. outgassing")) {
                RecalculateOutgassing(mApp);
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

    ProcessControlTable(mApp);

    ImGui::Text("Number of CPU cores:     %zd", mApp->numCPU);
    ImGui::Text("Number of subprocesses:  ");
    ImGui::SameLine();
    /*bool nbProcChanged = false;
    int nbProc = mApp->worker.GetProcNumber();*/
    ImGui::SetNextItemWidth(ImGui::CalcTextSize("0").x * 10);
    ImGui::InputInt("##nbProc", &nbProc, 1, 8);

    if (nbProc > MAX_PROCESS) {
        nbProc = MAX_PROCESS;
    }
    if (nbProc < 1) {
        nbProc = 1;
    }

    ImGui::SameLine();
    if (ImGui::Button("Apply and restart processes"))
        RestartProc(nbProc, mApp);
    {
        ImGui::PlaceAtRegionRight("Change desorption limit", true);
        if (ImGui::Button("Change desorption limit"))
            ImGui::OpenPopup("Edit MAX");
    }
    // Always center this window when appearing
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing,
                            ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal("Edit MAX", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {
        // static bool initMax = false;
        static double maxDes = mApp->worker.model->otfParams.desorptionLimit;
        /*if(!initMax) {
            maxDes = mApp->worker.model->otfParams.desorptionLimit; // use
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
            mApp->worker.model->otfParams.desorptionLimit = maxDes;
            // initMax = false;
            mApp->worker.ChangeSimuParams();
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

/**
 * \brief Function to apply changes to the number of processes.
 */
void RestartProc(int nbProc, Interface* mApp) {
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

void RecalculateOutgassing(Interface* mApp) {
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