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

#ifdef MOLFLOW
double ImSidebar::PumpingSpeedFromSticking(double sticking, double area, double temperature) {
    return 1.0 * sticking * area / 10.0 / 4.0 * sqrt(8.0 * 8.31 * temperature / PI / (mApp->worker.model->sp.gasMass * 0.001));
}

double ImSidebar::StickingFromPumpingSpeed(double pumpingSpeed, double area, double temperature) {
    return std::abs(pumpingSpeed / (area / 10.0) * 4.0 * sqrt(1.0 / 8.0 / 8.31 / (temperature) * PI * (mApp->worker.model->sp.gasMass * 0.001)));
}
#endif
void ImSidebar::DrawSectionDebug()
{
    if (drawn && ImGui::Button("Close this window"))
        drawn = false;
    if (ImGui::CollapsingHeader("[DEMO] Window flags")) {

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
}

void ImSidebar::DrawSectionViewerSettings()
{
    if (ImGui::CollapsingHeader("3D Viewer settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        auto curViewer = mApp->curViewer;
        auto viewer = mApp->viewers[curViewer];
        if (ImGui::BeginTable("table_3dviewer", 3, ImGuiTableFlags_None)) {
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
                if (ImGui::Button("<< View")) {
                    mApp->imWnd->viewSet.Toggle();
                    mApp->imWnd->viewSet.SetPos(ImVec2(pos.x-txtW*35, pos.y+txtH));
                }
                ImGui::TableNextColumn();
                ImGui::Checkbox("Indices", &viewer->showIndex);
                ImGui::TableNextColumn();
                ImGui::Checkbox("VertexIDs", &viewer->showVertexId);
            }
            ImGui::EndTable();
        }
    }
}

void ImSidebar::UpdateFacetSettings() {
    nbSelectedFacets = interfGeom->GetNbSelectedFacets();
    fSet = FacetSettings();
    if (nbSelectedFacets > 1) {
        title = fmt::format("Selected Facet ({} selected)##facetDetails", nbSelectedFacets);
    }
    else if (nbSelectedFacets == 1) {
        title = fmt::format("Selected Facet (#{})##facetDetails", interfGeom->GetSelectedFacets().front() + 1);
    }
    else {
        title = fmt::format("Selected Facet (none)##facetDetails");
        sel = nullptr;
        return;
    }
    selected_facet_id = nbSelectedFacets ? interfGeom->GetSelectedFacets().front() : 0;
    sel = nbSelectedFacets ? interfGeom->GetFacet(selected_facet_id) : nullptr;
    if (sel == nullptr) return;

    fSet.des_idx = sel->sh.desorbType;
    desorpComboContent = desorpComboOpts[fSet.des_idx];
    fSet.og = sel->sh.outgassing * 10; // from bar to mbar
    fSet.og_area = fSet.og / sel->sh.area;
    fSet.area = sel->sh.area;
    fSet.sf = sel->sh.sticking;
    fSet.sides_idx = sel->sh.is2sided;
    fSet.opacity = sel->sh.opacity;
    fSet.temp = sel->sh.temperature;
    fSet.prof_idx = sel->sh.profileType;
    profileComboContent = profileComboOpts[fSet.prof_idx];

    if (nbSelectedFacets > 1) {
        /*
        Loop over selection, replace differing values with '...' in interface
        */
        fSet.area = 0;
        fSet.psInput = "...";
        std::vector<size_t> facets = interfGeom->GetSelectedFacets();
        for (size_t& fID : facets) {
            InterfaceFacet& f = *interfGeom->GetFacet(fID);
            if (fSet.des_idx != f.sh.desorbType) desorpComboContent = "...";
            if (fSet.og != f.sh.outgassing * 10 || sel->sh.outgassingParam != f.sh.outgassingParam) {
                fSet.outgassingInput = "...";
                fSet.outgassingAreaInput = "...";
            }
            if (fSet.sf != f.sh.sticking || sel->sh.stickingParam != f.sh.stickingParam) {
                fSet.sfInput = "...";
                fSet.psInput = "...";
            }
            if (fSet.sides_idx != f.sh.is2sided) sidesComboContent = "...";
            if (fSet.opacity != f.sh.opacity || sel->sh.opacityParam != f.sh.opacityParam) fSet.opacityInput = "...";
            if (fSet.temp != f.sh.temperature || sel->sh.temperatureParam != f.sh.temperatureParam) fSet.temperatureInput = "...";
            fSet.area += f.sh.area;
            if (fSet.prof_idx != f.sh.profileType) profileComboContent = "...";
        }
    }
    // values that are not "..." can be filled now
    if (fSet.outgassingInput == "") {
        if (sel->sh.outgassingParam == "") fSet.outgassingInput = std::to_string(fSet.og);
        else fSet.outgassingInput = sel->sh.outgassingParam;
    }
    if (fSet.outgassingAreaInput == "") {
        fSet.outgassingAreaInput = std::to_string(fSet.og_area);
    }
    if (fSet.sfInput == "") {
        if (sel->sh.stickingParam == "") fSet.sfInput = std::to_string(fSet.sf);
        else fSet.sfInput = sel->sh.stickingParam;
    }
    fSet.ps = PumpingSpeedFromSticking(fSet.sf, sel->sh.area, sel->sh.temperature);
    if (fSet.psInput == "") fSet.psInput = std::to_string(fSet.ps);

    if (fSet.opacityInput == "") {
        if (sel->sh.opacityParam == "") fSet.opacityInput = std::to_string(fSet.opacity);
        else fSet.opacityInput = sel->sh.opacityParam;
    }
    if (fSet.temperatureInput == "") {
        if (sel->sh.temperatureParam == "") fSet.temperatureInput = std::to_string(fSet.temp);
        else fSet.temperatureInput = sel->sh.temperatureParam;
    }
    sidesComboContent = fmt::format("{} Sided", fSet.sides_idx+1);
}

void ImSidebar::ApplyFacetSettings() {
    // take values from nput fields and apply them to geometry facet

    LockWrapper lW(mApp->imguiRenderLock);
    if (!mApp->AskToReset()) return;

    std::vector<size_t> facets = interfGeom->GetSelectedFacets();
    for (size_t& fID : facets) {
        InterfaceFacet& f = *interfGeom->GetFacet(fID);

        if (desorpComboContent != "...")
        {
            f.sh.desorbType = fSet.des_idx;
            if (fSet.des_idx == 3) {
                if (fSet.exponentInput != "...") {
                    if (!Util::getNumber(&fSet.des_exponent, fSet.exponentInput)) {
                        ImIOWrappers::InfoPopup("Error", "Invalid desorption type exponent");
                        return;
                    }
                    else {
                        f.sh.desorbTypeN = fSet.des_exponent;
                    }
                }
            }
        }
        if (fSet.outgassingInput != "...") {
            if (!Util::getNumber(&fSet.og_area, fSet.outgassingInput)) {
                f.sh.outgassingParam = fSet.outgassingInput;
            }
            else {
                f.sh.outgassing = fSet.og / 10; // unit conversion
                f.sh.outgassingParam = "";
            }
        }
        if (fSet.sfInput != "...") {
            if (!Util::getNumber(&fSet.sf, fSet.sfInput)) {
                // input is NaN
                f.sh.stickingParam = fSet.sfInput;
            }
            else {
                // input is a valid number
                f.sh.sticking = fSet.sf;
                f.sh.stickingParam = "";
            }
        }
        if (sidesComboContent != "...") {
            f.sh.is2sided = fSet.sides_idx;
        }
        if (fSet.opacityInput != "...") {
            if (!Util::getNumber(&fSet.opacity, fSet.opacityInput)) {
                f.sh.opacityParam = fSet.opacityInput;
            }
            else {
                f.sh.opacity = fSet.opacity;
                f.sh.opacityParam = "";
            }
        }
        if (fSet.temperatureInput != "...") {
            if (!Util::getNumber(&fSet.temp, fSet.temperatureInput)) {
                f.sh.temperatureParam = fSet.temperatureInput;
            }
            else {
                f.sh.temperature = fSet.temp;
                f.sh.temperatureParam = "";
            }
        }
        mApp->changedSinceSave = true;

        if (profileComboContent != "...") {
            f.sh.profileType = fSet.prof_idx;
        }
        f.sh.maxSpeed = 4.0 * sqrt(2.0 * 8.31 * f.sh.temperature / 0.001 / mApp->worker.model->sp.gasMass);

        f.UpdateFlags();
    }
    mApp->worker.MarkToReload();
    fSet.facetSettingsChanged = false;
    
    mApp->ImRefresh();
}

void ImSidebar::DrawSectionSelectedFacet()
{
    // remember last state to see if values need updating
    if (ImGui::CollapsingHeader(title.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
        if (sel == nullptr) ImGui::BeginDisabled();
#if defined(MOLFLOW)
        if (ImGui::TreeNodeEx("Particles in", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::AlignTextToFramePadding();
            ImGui::Text("Desorption"); ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 10);
            if (ImGui::BeginCombo("##Desorption", desorpComboContent)) {
                // change from original: add ... as a combo option when multiple facets selected
                // to explicidly decide to not change the desorption type
                if (nbSelectedFacets > 1) {
                    if (ImGui::Selectable("...")) {
                        desorpComboContent = "...";
                    }
                }
                for (unsigned short i = 0; i < 5; i++) {
                    if (ImGui::Selectable(desorpComboOpts[i])) {
                        fSet.des_idx = i;
                        desorpComboContent = desorpComboOpts[i];
                        fSet.facetSettingsChanged = true;
                    }
                }
                ImGui::EndCombo();
            }
            if (fSet.des_idx == 3 && desorpComboContent != "...") {
                ImGui::SameLine();
                ImGui::SetNextItemWidth(txtW * 6);
                if (ImGui::InputText("##exponent", &fSet.exponentInput)) fSet.facetSettingsChanged = true;
            }
            if (fSet.des_idx == 0 || desorpComboContent == "...") ImGui::BeginDisabled();

            if (ImGui::RadioButton("##OutGas", fSet.modeOfOg == fSet.use_og)) {
                fSet.modeOfOg = fSet.use_og;
            }
            ImGui::SameLine();
            ImGui::TextWithMargin(u8"Outgassing [mbar\u00b7l/s]", txtW * 18);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##in", &fSet.outgassingInput)) {
                fSet.modeOfOg = fSet.use_og;
                // if value changed
                if (Util::getNumber(&fSet.og, fSet.outgassingInput)) { // and conversion was a success
                    //update the other value
                    fSet.og_area = fSet.og / fSet.area;
                    fSet.outgassingAreaInput = std::to_string(fSet.og_area);
                }
                fSet.facetSettingsChanged = true;
            }
            if (ImGui::RadioButton("##OutGasArea", fSet.modeOfOg == fSet.use_og_area)) {
                fSet.modeOfOg = fSet.use_og_area;
            }
            ImGui::SameLine();
            ImGui::TextWithMargin(u8"Outg/area [mbar\u00b7l/s/cm\u00b2]", txtW * 18);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##ina", &fSet.outgassingAreaInput)) {
                fSet.modeOfOg = fSet.use_og_area;
                if (Util::getNumber(&fSet.og_area, fSet.outgassingAreaInput)) {
                    fSet.og = fSet.og_area * fSet.area;
                    fSet.outgassingInput = std::to_string(fSet.og);
                }
                fSet.facetSettingsChanged = true;
            }
            if (fSet.des_idx == 0 || desorpComboContent == "...") ImGui::EndDisabled();
            ImGui::TreePop();
        }
        if (ImGui::TreeNodeEx("Particles out", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::TextWithMargin("Sticking Factor", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##StickingFactor", &fSet.sfInput)) {
                if (Util::getNumber(&fSet.sf, fSet.sfInput)) {
                    // could work incorrectly correctly if area and temperature are changed
                    fSet.ps = PumpingSpeedFromSticking(fSet.sf, fSet.area, fSet.temp);
                    fSet.psInput = std::to_string(fSet.ps);
                }
                fSet.facetSettingsChanged = true;
            }
            if (nbSelectedFacets > 1) ImGui::BeginDisabled();
            ImGui::TextWithMargin("Pumping speed [l/s]", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##PumpingSpeed", &fSet.psInput)) {
                if (Util::getNumber(&fSet.ps, fSet.psInput)) {
                    // could work incorrectly correctly if area and temperature are changed
                    fSet.sf = StickingFromPumpingSpeed(fSet.ps, fSet.area, fSet.temp);
                    fSet.sfInput = std::to_string(fSet.sf);
                }
                fSet.facetSettingsChanged = true;
            }
            if (nbSelectedFacets > 1) ImGui::EndDisabled();
            if (fSet.sf < 0 || fSet.sf > 1) { // if sf is out of range display warning
                ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"Sticking factor \u2209[0,1]");
            }
            ImGui::TreePop();
        }
#endif
        {
            ImGui::AlignTextToFramePadding();
            ImGui::TextWithMargin("Sides", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##Sides", sidesComboContent)) {
                if (nbSelectedFacets > 1) {
                    if (ImGui::Selectable("...")) {
                        // explicidly choose to not change facet sidedness
                        sidesComboContent = "...";
                    }
                }
                if (ImGui::Selectable("1 Sided")) {
                    sidesComboContent = "1 Sided";
                    fSet.sides_idx = 0;
                    fSet.facetSettingsChanged = true;
                }
                if (ImGui::Selectable("2 Sided")) {
                    sidesComboContent = "2 Sided";
                    fSet.sides_idx = 1;
                    fSet.facetSettingsChanged = true;
                }
                ImGui::EndCombo();
            }
            ImGui::TextWithMargin("Opacity", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##Opacity", &fSet.opacityInput)) {
                Util::getNumber(&fSet.opacity, fSet.opacityInput); // converto to number so the warning can be displayed
                fSet.facetSettingsChanged = true;
            }
            if (fSet.opacity < 0 || fSet.opacity > 1) { // if opacity is out of range display the warining
                ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"Opacity \u2209[0,1]");
            }

#if defined(MOLFLOW)
            ImGui::TextWithMargin(u8"Temperature [\u212a]", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::InputText("##Temperature", &fSet.temperatureInput)) {
                Util::getNumber(&fSet.temp, fSet.temperatureInput);
                fSet.facetSettingsChanged = true;
            }
            if (fSet.temp <= 0) {
                ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"Temperature must be positive (\u212a)");
            }
#endif
            {
                if (nbSelectedFacets > 1) {
                    ImGui::TextWithMargin(u8"Sum area [cm\u00b2]", txtW * 14);
                }
                else {
                    ImGui::TextWithMargin(u8"Area [cm\u00b2]", txtW * 14);
                }
                ImGui::SameLine();
                ImGui::SetNextItemWidth(-1);
                ImGui::BeginDisabled();
                ImGui::InputDouble("##Area", &fSet.area);
                ImGui::EndDisabled();
            }
            ImGui::TextWithMargin("Profile", txtW * 14); ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##Profile", profileComboContent)) {
                if (nbSelectedFacets > 1) {
                    if (ImGui::Selectable("...")) {
                        profileComboContent = "...";
                    }
                }
                for (unsigned short i = 0; i < 7; i++) {
                    if (ImGui::Selectable(profileComboOpts[i])) {
                        fSet.prof_idx = i;
                        profileComboContent = profileComboOpts[i];
                        fSet.facetSettingsChanged = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
        if (sel == nullptr) ImGui::EndDisabled();
        if (ImGui::Button("<<Adv.")) mApp->imWnd->advFacPar.Show(); // TODO open advanced facet params window
        ImGui::SameLine();
        if (ImGui::Button("Details")) {} // TODO open facet details window
        ImGui::SameLine();
        if (ImGui::Button("Coords")) mApp->imWnd->facCoord.Toggle();
        ImGui::SameLine();
        bool disabled = !fSet.facetSettingsChanged;
        if (disabled) ImGui::BeginDisabled();
        if (ImGui::Button("Apply")) {
            ApplyFacetSettings();
        }
        if (disabled) ImGui::EndDisabled();
    }
}

void ImSidebar::UpdateSimulationData() {
    runningState = mApp->worker.IsRunning();
    if ((mApp->worker.simuTimer.Elapsed() <= 2.0f) && runningState) {
        hit_stat = "Starting...";
        des_stat = "Starting...";
    }
    else {
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
        if (mApp->worker.globalStatCache.nbLeakTotal) {
            leak_stat = fmt::format("{} ({})", mApp->worker.globalStatCache.nbLeakTotal,
                mApp->worker.globalStatCache.nbLeakTotal * 100 / mApp->worker.globalStatCache.globalHits.nbDesorbed);
        }
        else {
            leak_stat = "None";
        }
    }
    time_stat = fmt::format("{}: {}", runningState ? "Running" : "Stopped", Util::formatTime(mApp->worker.simuTimer.Elapsed()));
}

void ImSidebar::DrawSectionSimulation()
{
    if (ImGui::CollapsingHeader("Simulation", ImGuiTreeNodeFlags_DefaultOpen)) {
        if (ImGui::Button("<< Sim")) {
            mApp->imWnd->globalSet.Toggle();
        }
        ImGui::SameLine();
        {
            if (mApp->worker.IsRunning()) {
                simBtnLabel = fmt::format("Pause");
            }
            else if (mApp->worker.globalStatCache.globalHits.nbMCHit > 0) {
                simBtnLabel = fmt::format("Resume");
            }
            else {
                simBtnLabel = fmt::format("Begin");
            }
        }
        if (ImGui::Button(simBtnLabel.c_str())) {
            mApp->changedSinceSave = true;
            if (molApp != nullptr) {
                LockWrapper lW(mApp->imguiRenderLock);
                molApp->StartStopSimulation();
            }
        }
        ImGui::SameLine();
        bool disable = mApp->worker.globalStatCache.globalHits.nbDesorbed <= 0;
        if (disable) ImGui::BeginDisabled();
        if (ImGui::Button("Reset")) {
            mApp->changedSinceSave = true;
            LockWrapper lW(mApp->imguiRenderLock);
            mApp->ResetSimulation();
        }
        if (disable) ImGui::EndDisabled();

        ImGui::Checkbox("Auto update scene", &mApp->autoFrameMove);
        if (mApp->autoFrameMove) {
            ImGui::BeginDisabled();
        }
        ImGui::SameLine();
        if (ImGui::Button("Update")) {
            mApp->updateRequested = true;
        }
        if (mApp->autoFrameMove) {
            ImGui::EndDisabled();
        }

        ImGui::TextWithMargin("Hits: ", txtW*10);
        ImGui::SameLine();
        ImGui::Text(fmt::format("{}", hit_stat));
        ImGui::TextWithMargin("Des: ", txtW*10);
        ImGui::SameLine();
        ImGui::Text(fmt::format("{}", des_stat));
        ImGui::TextWithMargin("Leaks: ", txtW*10);
        ImGui::SameLine();
        ImGui::Text(fmt::format("{}", leak_stat));
        ImGui::TextWithMargin("Time: ", txtW*10);
        ImGui::SameLine();
        ImGui::Text(time_stat);
    }
}

void ImSidebar::DrawFacetTable()
{
    try {
        // Facet list
        if (interfGeom->IsLoaded()) {
            // TODO: Colors for moment highlighting
            // Create item list that is sortable etc.

            static ImGuiTableFlags tFlags =
                ImGuiTableFlags_ScrollY | ImGuiTableFlags_SizingFixedFit |
                /*ImGuiTableFlags_RowBg | */ImGuiTableFlags_BordersOuter |
                ImGuiTableFlags_BordersV | ImGuiTableFlags_Resizable |
                ImGuiTableFlags_Reorderable | ImGuiTableFlags_Hideable |
                ImGuiTableFlags_Sortable;

            ImVec2 outer_size = ImVec2(0.0f, std::max(ImGui::GetContentRegionAvail().y, txtH * 8.f));
            if (ImGui::BeginTable("facetlist", 4, tFlags, outer_size)) {
                ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
                ImGui::TableSetupColumn("#", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_ID);
                ImGui::TableSetupColumn("Hits", ImGuiTableColumnFlags_WidthFixed, 0.0f, FacetDataColumnID_Hits);
                ImGui::TableSetupColumn("Des", ImGuiTableColumnFlags_WidthStretch, 0.0f, FacetDataColumnID_Des);
                ImGui::TableSetupColumn("Abs", ImGuiTableColumnFlags_WidthStretch, 0.0f, FacetDataColumnID_Abs);
                ImGui::TableHeadersRow();

                // Sort our data if sort specs have been changed!
                if (ImGuiTableSortSpecs* sorts_specs = ImGui::TableGetSortSpecs())
                    if (sorts_specs->SpecsDirty) {
                        FacetData::s_current_sort_specs = sorts_specs; // Store in variable accessible by the sort function.
                        if (items.Size > 1)
                            qsort(&items[0], (size_t)items.Size, sizeof(items[0]),
                                FacetData::CompareWithSortSpecs);
                        FacetData::s_current_sort_specs = nullptr;
                        sorts_specs->SpecsDirty = false;
                    }

                // Demonstrate using clipper for large vertical lists
                ImGuiListClipper clipper;
                clipper.Begin(items.size());
                while (clipper.Step()) {
                    for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++) {
                        FacetData &item = items[i];
                        //ImGui::PushID(item->ID);
                        ImGui::TableNextRow();
                        ImGui::TableSetColumnIndex(0);
                        if (ImGui::Selectable(fmt::format("{}", item.ID+1), item.selected, ImGuiSelectableFlags_SpanAllColumns)) {
                            if (!ImGui::IsKeyDown(ImGuiKey_LeftCtrl)) { // if ctrl not held
                                item.selected = true;
                                for (int j = 0; j < items.Size; j++) {
                                    if (items[j].ID != item.ID)
                                        items[j].selected = false; // deselect all others
                                }
                            }
                            else { // if ctrl is pressed
                                // do not change the state of others
                                item.selected = !item.selected; // and invert the clicked
                            }
                            UpdateSelectionFromTable(); // apply the changes in the actual geometry
                        }
                        ImGui::TableNextColumn();
                        ImGui::Text("%zd", item.hits);
                        ImGui::TableNextColumn();
                        ImGui::Text("%zd", item.des);
                        ImGui::TableNextColumn();
                        ImGui::Text("%g", item.abs);
                        //ImGui::PopID();
                    }
                }
                ImGui::EndTable();
            }

        }
    }
    catch (const std::exception& e) {
        char errMsg[512];
        sprintf(errMsg, "%s\nError while updating facet hits", e.what());
        /*GLMessageBox::Display(errMsg, "Error", GLDLG_OK, GLDLG_ICONERROR);*/
    }
}

void ImSidebar::UpdateTable()
{
    if (items.Size != interfGeom->GetNbFacet()) {
        items.resize(static_cast<int>(interfGeom->GetNbFacet()), FacetData());
    }
    LockWrapper lW(mApp->imguiRenderLock);
    for (int n = 0; n < items.Size; n++) {
        InterfaceFacet* f = interfGeom->GetFacet(n);
        FacetData& item = items[n];
        item.ID = n;
        item.hits = f->facetHitCache.nbMCHit;
        item.des = f->facetHitCache.nbDesorbed;
        item.abs = f->facetHitCache.nbAbsEquiv;
        item.selected = f->selected;
    };
}

void ImSidebar::UpdateSelectionFromTable(bool shift, bool ctrl)
{
    // when selection inside the sidebar table changes update geometry selection to match
    std::vector<size_t> newSelection;
    for (const FacetData &f : items) {
        if (f.selected) newSelection.push_back(f.ID);
    }
    interfGeom->SetSelection(newSelection, shift, ctrl);
    interfGeom->UpdateSelection();
}

void ImSidebar::Draw() {
    if (!drawn) return;
    const ImGuiViewport* viewport = ImGui::GetMainViewport();
    float width = txtW * 40;
    // set size and pos on every draw so it updates when window resizes
    pos = ImVec2(viewport->Size.x - width, viewport->WorkPos.y);
    ImGui::SetNextWindowPos(pos);
    size = ImVec2(width, viewport->WorkSize.y);
    ImGui::SetNextWindowSize(size);

    ImGui::Begin("[BETA] Molflow Sidebar", &drawn, flags);
#if defined(DEBUG)
    DrawSectionDebug();
#endif
    DrawSectionViewerSettings();
    DrawSectionSelectedFacet();
    DrawSectionSimulation();
    DrawFacetTable();
    ImGui::End();
}

void ImSidebar::Init(Interface* mApp_)
{
    mApp = mApp_;
    interfGeom = mApp->worker.GetGeometry();
    molApp = dynamic_cast<MolFlow*>(mApp);
}

void ImSidebar::OnShow()
{
#ifndef DEBUG
    ImIOWrappers::InfoPopup("WARNING", "Sidebar is not properly implemented yet. Most features do not work!");
#endif
    Update();
}

void ImSidebar::Update()
{
    UpdateTable();
    UpdateFacetSettings();
    UpdateSimulationData();
}
