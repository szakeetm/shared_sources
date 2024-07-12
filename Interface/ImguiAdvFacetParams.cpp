#include "ImguiAdvFacetParams.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"
#include "ImguiWindow.h"

#ifdef MOLFLOW
#include "MolFlow.h"
#endif

void ImAdvFacetParams::Draw()
{
    if (!drawn) return;
    ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 55, txtH * 10), ImVec2(txtW * 500, txtH * 100));
	ImGui::Begin("Advanced Facet Parameters", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
    if (nbSelected == 0) ImGui::BeginDisabled();
    if (ImGui::CollapsingHeader("Texture properties"))
    {
        if (ImGui::TriState("Enable texture", &enableTexture, enableTextureAllowMixed))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
        ImGui::SameLine();
        if (ImGui::TriState("Use square cells", &useSquareCells, useSquareCellsAllowMixed))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
        ImGui::SameLine();
        if (ImGui::Button("Force remesh")) ForceRemeshButtonPress();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Resolution:");
        ImGui::SetNextItemWidth(txtW * 6);
        if (ImGui::InputText("##ResolutionU", &resolutionUIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            enableTexture = true;
            if (Util::getNumber(&resolutionU, resolutionUIn)) {
                cellSizeU = 1.0 / resolutionU;
                cellSizeUIn = fmt::format("{}", cellSizeU);
                cellsUIn = "";
            }
            else {
                cellSizeUIn = "";
            }
            if (useSquareCells) {
                resolutionVIn = resolutionUIn;
                resolutionV = resolutionU;
                cellSizeVIn = cellSizeUIn;
                cellSizeV = cellSizeU;
            }
            UpdateCellCount();
            UpdateMemoryEstimate();
        }
        if (!useSquareCells) {
            ImGui::SameLine();
            ImGui::Text("x");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 6);
            if (ImGui::InputText("###ResolutionV", &resolutionVIn)) {
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
                enableTexture = true;
                if (Util::getNumber(&resolutionV, resolutionVIn)) {
                    cellSizeVIn = fmt::format("{}", 1.0 / resolutionV);
                }
                else {
                    cellSizeVIn = "";
                }
                UpdateCellCount();
                UpdateMemoryEstimate();
            }
        }
        ImGui::SameLine();
        ImGui::Text("cells/cm");
        ImGui::SetNextItemWidth(txtW * 6);
        if (ImGui::InputText("##sizeU", &cellSizeUIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            if (Util::getNumber(&cellSizeU, cellSizeUIn)) {
                resolutionU = 1.0 / cellSizeU;
                resolutionUIn = fmt::format("{}", resolutionU);
            }
            else {
                resolutionUIn = "";
            }
            if (useSquareCells) {
                cellSizeV = cellSizeU;
                cellSizeVIn = cellSizeUIn;
                resolutionV = resolutionU;
                resolutionVIn = resolutionUIn;
            }
            UpdateCellCount();
            UpdateMemoryEstimate();
        }
        if (!useSquareCells) {
            ImGui::SameLine();
            ImGui::Text("x");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 6);
            if (ImGui::InputText("###sizeV", &cellSizeVIn)) {
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
                enableTexture = true;
                if (Util::getNumber(&cellSizeV, cellSizeVIn)) {
                    resolutionVIn = fmt::format("{}", 1.0 / cellSizeV);
                }
                else {
                    resolutionVIn = "";
                }
                UpdateCellCount();
                UpdateMemoryEstimate();
            }
        }
        ImGui::SameLine();
        ImGui::Text("cm/cell");

        if (nbSelected != 1) ImGui::BeginDisabled();

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Number of cells:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        if (ImGui::InputText("##CellsU", &cellsUIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            EditCellCount();
        }
        ImGui::SameLine();
        ImGui::Text("x");
        if (useSquareCells == 1) ImGui::BeginDisabled();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::SameLine();
        if (ImGui::InputText("##CellsV", &cellsVIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            EditCellCount();
        }
        if (useSquareCells == 1) ImGui::EndDisabled();

        if (nbSelected != 1) ImGui::EndDisabled();

        if (ImGui::BeginTable("##AFPLayoutHelper", 2)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            if (ImGui::TriState("Count desorption", &countDesorption, countDesorptionAllowMixed))
            {
                if (countDesorption == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            if (ImGui::TriState("Count absorption", &countAbsorption, countAbsorptionAllowMixed))
            {
                if (countAbsorption == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            if (ImGui::TriState("Angular coefficient", &angularCoefficient, angularCoefficientAllowMixed))
            {
                if (angularCoefficient == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            ImGui::TableNextColumn();
            if (ImGui::TriState("Count reflection", &countReflection, countReflectionAllowMixed))
            {
                if (countReflection == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            if (ImGui::TriState("Count transparent pass", &countTransparentPass, countTransparentPassAllowMixed))
            {
                if (countTransparentPass == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            if (ImGui::TriState("Record direction vectors", &recordDirectionVectors, recordDirectionVectorsAllowMixed))
            {
                if (recordDirectionVectors == 1 && enableTexture == 0) enableTexture = 1;
                mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            }
            ImGui::EndTable();
        }

    }

    if (ImGui::CollapsingHeader("Texture cell / memory"))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text(fmt::format("Memory:{}\tCells:{}", memory,cells));
    }

    if (ImGui::CollapsingHeader("Additional parameters"))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Reflection:", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        if (ImGui::InputText("##diffuseIn", &diffuseIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            if (Util::getNumber(&diffuse, diffuseIn)) {
                cosine = 1.0 - diffuse - specular;
                cosineIn = fmt::format("{:.3g}", cosine);
            }
        }
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::TextWithMargin("part diffuse,", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        if (ImGui::InputText("##specularIn", &specularIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            if (Util::getNumber(&specular, specularIn)) {
                cosine = 1.0 - diffuse - specular;
                cosineIn = fmt::format("{:.3g}", cosine);
            }
        }
        ImGui::SameLine();
        ImGui::Text("part specular,");
        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin(" \t", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        // in legacy the input into this box is not used anywhere
        // it is just displaying information so to not confuse users
        // I made it disabled
        ImGui::BeginDisabled();
        ImGui::InputText("##cosineIn", &cosineIn);
        ImGui::EndDisabled();

        ImGui::SameLine();
        ImGui::TextWithMargin("part cosine^", txtW * 10);
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::SameLine();
        if (ImGui::InputText("##cosineNIn", &reflextionExponentIn))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;

        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Accomodation coefficient:", txtW * 20);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        if(ImGui::InputText("##Accommodation", &accomodationIn))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Teleport to facet:", txtW * 20);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        if (ImGui::InputText("##TeleportTo", &teleportIn))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;


        ImGui::AlignTextToFramePadding();
        ImGui::Text("Structure:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        if (ImGui::InputText("##Structure", &structureIn))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
        ImGui::SameLine();
        ImGui::Text("Link to:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        if (ImGui::InputText("##LinkTo", &linkIn))
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;

        if (ImGui::TriState("Moving part", &movingPart, movingPartAllowMixed)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
        }
        if (ImGui::TriState(fmt::format("{}###Wall sojourn time", sojournText).c_str(), &wallSojourn, wallSojournAllowMixed)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            CalcSojournTime();
        }
        ImGui::SameLine();
        ImGui::HelpMarker(R"(Sojourn time calculated by Frenkel's equation

f: Molecule's surface oscillation frequency [Hz]
E: Adsorption energy [J/mole]
A: Escape probability per oscillation:
A = exp(-E/(R*T))

Probability of sojourn time t:
p(t)= A*f*exp(-A*f*t)

Mean sojourn time:
mean= 1/(A*f) = 1/f*exp(E/(kT))

More info: read report CERN-OPEN-2000-265
from C. Benvenutti http://cds.cern.ch/record/454180
)");
        if (wallSojourn == 0) ImGui::BeginDisabled();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Attempt freq:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        if (ImGui::InputText("##AttemptFreq", &freqIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            if (Util::getNumber(&freq, freqIn)) {
                CalcSojournTime();
            }
        }
        ImGui::SameLine();
        ImGui::Text("Hz\t Binding E:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        if (ImGui::InputText("##BindingE", &bindingIn)) {
            mApp->imWnd->sideBar.fSet.facetSettingsChanged = true;
            if (Util::getNumber(&binding, bindingIn)) {
                CalcSojournTime();
            }
        }
        ImGui::SameLine();
        ImGui::Text("J/mole");
        if (wallSojourn == 0) ImGui::EndDisabled();
    }

    if (ImGui::CollapsingHeader("View settings"))
    {
        ImGui::TriState("Draw Texture", &drawTexture, drawTextureAllowMixed);
        ImGui::SameLine();
        ImGui::TriState("Draw Volume", &drawVolume, drawVolumeAllowMixed);
        ImGui::SameLine();
        if (ImGui::Button("<- Change draw")) ApplyDrawSettings();
    }

    if (ImGui::CollapsingHeader("Dynamic desorption"))
    {
        if (ImGui::BeginTable("##DeynDesorpLayoutHelper", 2)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::AlignTextToFramePadding();
            ImGui::TextWithMargin("Use file:", txtW * 6);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(-1);
            if (ImGui::BeginCombo("##UseFile", "")) {
                ImGui::EndCombo();
            }
            ImGui::TextWithMargin("Avg", txtW * 6);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 10);
            ImGui::InputText("##Avg2", &avg2In);
            ImGui::SameLine();
            ImGui::TextWithMargin("ph/s/cm2", txtW * 5);
            
            ImGui::TableNextColumn();

            ImGui::Text("Avg");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 10);
            ImGui::InputText("##Avg1", &avg1In);
            ImGui::SameLine();
            ImGui::Text("mol/ph");

            ImGui::AlignTextToFramePadding();
            ImGui::Text("Avg");
            ImGui::SameLine();
            ImGui::SetNextItemWidth(txtW * 10);
            ImGui::InputText("##Avg3", &avg3In);
            ImGui::SameLine();
            ImGui::Text("pg/cm2");

            ImGui::EndTable();
        }
    }

    if (ImGui::CollapsingHeader("Incident angle distribution"))
    {
        ImGui::Checkbox("Record", &record);

        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Theta (grazing angle):", txtW * 15);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##Theta", &thetaIn);
        ImGui::SameLine();
        ImGui::TextWithMargin("values from 0 to", txtW * 11);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##max", &maxIn);

        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("\t", txtW * 15);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##nVals", &nValsIn);
        ImGui::SameLine();
        ImGui::Text("values from limit to PI/2");

        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Phi (azimith with U):", txtW * 15);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##Phi", &phiIn);
        ImGui::SameLine();
        ImGui::Text("values from -PI to +PI");

        if (ImGui::Button("Copy")) CopyButtonPress();
        ImGui::SameLine();
        if (ImGui::Button("Export to CSV")) ExportToCSVButtonPress();
        ImGui::SameLine();
        if (ImGui::Button("Import CSV")) ImportCVSButtonPress();
        ImGui::SameLine();
        if (ImGui::Button("Release recorded")) ReleaseRecordedButtonPress();
    }
    if (nbSelected == 0) ImGui::EndDisabled();
    ImGui::Separator();
    if (ImGui::Button("Apply all")) mApp->imWnd->sideBar.ApplyFacetSettings(); // sidebars apply calls this class's apply
    ImGui::End();
}

void ImAdvFacetParams::Update()
{
    nbSelected = interfGeom->GetNbSelectedFacets();
    if (nbSelected == 0) {
        return;
    }
    std::vector<size_t> selected = interfGeom->GetSelectedFacets();
    InterfaceFacet* f0 = interfGeom->GetFacet(selected[0]);
    // draw toggles
    drawTexture = f0->viewSettings.textureVisible;
    drawTextureAllowMixed = false;
    drawVolume = f0->viewSettings.volumeVisible;
    drawVolumeAllowMixed = false;

    // texture properties
    enableTexture = f0->sh.isTextured;
    enableTextureAllowMixed = false;
    useSquareCells = std::abs(f0->tRatioU - f0->tRatioV) < 1E-8 ? 1 : 0; // 1 - true, 0 - false
    useSquareCellsAllowMixed = false;
    countDesorption = f0->sh.countDes;
    countDesorptionAllowMixed = false;
    countReflection = f0->sh.countRefl;
    countReflectionAllowMixed = false;
    countAbsorption = f0->sh.countAbs;
    countAbsorptionAllowMixed = false;
    countTransparentPass = f0->sh.countTrans;
    countTransparentPassAllowMixed = false;
    angularCoefficient = f0->sh.countACD;
    angularCoefficientAllowMixed = false;
    recordDirectionVectors = f0->sh.countDirection;
    recordDirectionVectorsAllowMixed = false;

    resolutionU = f0->tRatioU;
    resolutionUIn = fmt::format("{}", resolutionU);
    resolutionV = f0->tRatioV;
    resolutionVIn = fmt::format("{}", resolutionV);
    auto nbCells = f0->GetNbCellForRatio(resolutionU, resolutionV);
    cellsU = nbCells.first;
    cellsUIn = fmt::format("{}", cellsU);
    cellsV = nbCells.second;
    cellsVIn = fmt::format("{}", cellsV);
    cellSizeU = 1.0 / resolutionU;
    cellSizeUIn = fmt::format("{}", cellsU);
    cellSizeV = 1.0 / resolutionV;
    cellSizeVIn = fmt::format("{}", cellsV);

    wallSojourn = f0->sh.enableSojournTime;
    wallSojournAllowMixed = false;
    freq = f0->sh.sojournFreq;
    freqIn = fmt::format("{:.5g}", freq);
    binding = f0->sh.sojournE;
    bindingIn = fmt::format("{:.5g}", binding);

    diffuse = f0->sh.reflection.diffusePart;
    diffuseIn = fmt::format("{}", diffuse);
    specular = f0->sh.reflection.specularPart;
    specularIn = fmt::format("{}", specular);
    cosine = 1.0 - diffuse - specular;
    cosineIn = fmt::format("{}", cosine);
    reflextionExponent = f0->sh.reflection.cosineExponent;
    reflextionExponentIn = fmt::format("{}", reflextionExponent);

    accomodation = f0->sh.accomodationFactor;
    accomodationIn = fmt::format("{}", accomodation);
    teleport = f0->sh.teleportDest;
    teleportIn = fmt::format("{}", teleport);
    structure = f0->sh.superIdx;
    structureIn = structure == -1 ? "All" : fmt::format("{}", structure + 1);
    link = f0->sh.superDest;
    linkIn = link == 0 ? "No" : fmt::format("{}", link);

    movingPart = f0->sh.isMoving;

    for (int i = 0; i < selected.size(); i++) {
        InterfaceFacet* f = interfGeom->GetFacet(selected[i]);
        // draw toggles
        if (drawTexture != f->viewSettings.textureVisible) {
            drawTexture = 2;
            drawTextureAllowMixed = true;
        }
        if (drawVolume != f->viewSettings.volumeVisible) {
            drawVolume = 2;
            drawVolumeAllowMixed = true;
        }
        // texture properties
        if (enableTexture != f->sh.isTextured) {
            enableTexture = 2;
            enableTextureAllowMixed = true;
        }
        if (useSquareCells != std::abs(f->tRatioU - f->tRatioV) < 1E-8 ? 1 : 0) {
            useSquareCells = 2;
            useSquareCellsAllowMixed = true;
        }
        if (countDesorption != f->sh.countDes) {
            countDesorption = 2;
            countDesorptionAllowMixed = true;
        }
        if (countReflection != f->sh.countRefl) {
            countReflection = 2;
            countReflectionAllowMixed = true;
        }
        if (countAbsorption != f->sh.countAbs) {
            countAbsorption = 2;
            countAbsorptionAllowMixed = true;
        }
        if (countTransparentPass != f->sh.countTrans) {
            countTransparentPass = 2;
            countTransparentPassAllowMixed = true;
        }
        if (angularCoefficient != f->sh.countACD) {
            angularCoefficient = 2;
            angularCoefficientAllowMixed = true;
        }
        if (recordDirectionVectors != f->sh.countDirection) {
            recordDirectionVectors = 2;
            recordDirectionVectorsAllowMixed = true;
        }
        if (resolutionU != f->tRatioU || resolutionV != f->tRatioV) {
            resolutionUIn = "...";
            resolutionVIn = "...";
            cellSizeUIn = "...";
            cellSizeVIn = "...";
            cellsUIn = "...";
            cellsVIn = "...";
        }
        if (wallSojourn != f->sh.enableSojournTime) {
            wallSojourn = 2;
            wallSojournAllowMixed = true;
        }
        if (freq != f->sh.sojournFreq) {
            freqIn = "...";
        }
        if (binding != f->sh.sojournE) {
            bindingIn = "...";
        }
        if (diffuse != f->sh.reflection.diffusePart) {
            diffuseIn = "...";
        }
        if (specular != f->sh.reflection.specularPart) {
            specularIn = "...";
        }
        if (!IsEqual(cosine, 1.0 - f->sh.reflection.diffusePart - f->sh.reflection.specularPart)) {
            cosineIn = "...";
        }
        if (reflextionExponent != f->sh.reflection.cosineExponent) {
            reflextionExponentIn = "...";
        }
        if (accomodation != f->sh.accomodationFactor) {
            accomodationIn = "...";
        }
        if (teleport != f->sh.teleportDest) {
            teleportIn = "...";
        }
        if (structure != f->sh.superIdx) {
            structureIn = "...";
        }
        if (link != f->sh.superDest) {
            linkIn = "...";
        }
        if (movingPart != f->sh.isMoving) {
            movingPart = 2;
            movingPartAllowMixed = true;
        }
    }
    if (enableTexture == 0) { // none of the facets have textures
        resolutionUIn = "";
        resolutionVIn = "";
        cellSizeUIn = "";
        cellSizeVIn = "";
        cellsUIn = "";
        cellsVIn = "";
    }
    UpdateMemoryEstimate();
    CalcSojournTime();
}

void ImAdvFacetParams::ApplyDrawSettings()
{
    for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
        InterfaceFacet* f = interfGeom->GetFacet(i);
        if (f->selected) {

            if (drawTexture < 2) f->viewSettings.textureVisible = drawTexture;
            if (drawVolume < 2) f->viewSettings.volumeVisible = drawVolume;
        }
    }
    LockWrapper lW(mApp->imguiRenderLock);
    interfGeom->BuildGLList(); //Re-render facets
}

void ImAdvFacetParams::ExportToCSVButtonPress()
{
    LockWrapper lW(mApp->imguiRenderLock);
#ifdef MOLFLOW
    dynamic_cast<MolFlow*>(mApp)->ExportAngleMaps();
#endif
}

void ImAdvFacetParams::CopyButtonPress()
{
    LockWrapper lW(mApp->imguiRenderLock);
#ifdef MOLFLOW
    dynamic_cast<MolFlow*>(mApp)->CopyAngleMapToClipboard();
#endif
}

void ImAdvFacetParams::ReleaseRecordedButtonPress()
{
    LockWrapper lW(mApp->imguiRenderLock);
#ifdef MOLFLOW
    dynamic_cast<MolFlow*>(mApp)->ClearAngleMapsOnSelection();
#endif
    Update();
}

void ImAdvFacetParams::ImportCVSButtonPress()
{
    LockWrapper lW(mApp->imguiRenderLock);
#ifdef MOLFLOW
    dynamic_cast<MolFlow*>(mApp)->ImportAngleMaps();
#endif
    Update();
}

void ImAdvFacetParams::ForceRemeshButtonPress()
{
    LockWrapper lW(mApp->imguiRenderLock);
    ApplyTexture(true);
    mApp->worker.MarkToReload();
}

void ImAdvFacetParams::ApplyTexture(bool force)
{
    bool boundMap = true; // boundaryBtn->GetState();
    auto selectedFacets = interfGeom->GetSelectedFacets();
    int nbPerformed = 0;
    bool doRatio = false;

    if (enableTexture) { //check if valid texture settings are to be applied

        // Check counting mode
        if (!countDesorption && !countAbsorption &&
            !countReflection && !countTransparentPass &&
            !angularCoefficient && !recordDirectionVectors) {
            ImIOWrappers::InfoPopup("Error", "Please select counting mode");
            return;
        }

        // Resolution
        if (Util::getNumber(&resolutionU, resolutionUIn) && Util::getNumber(&resolutionV, resolutionVIn) && resolutionU>=0.0 && resolutionV>=0) {
            //Got a valid number
            doRatio = true;
        }
        else if (resolutionUIn != "..." || resolutionVIn != "...") { //Not in mixed "..." state
            ImIOWrappers::InfoPopup("Error", "Invalid texture resolution\nMust be a non-negative number");
            return;
        }
        else {
            //Mixed state: leave doRatio as false
        }
    }

    if (!mApp->AskToReset(&mApp->worker)) return;
    //auto prg = GLProgress_GUI("Applying mesh settings", "Please wait");
    //prg.SetVisible(true);
    //int count = 0;
    for (auto& sel : selectedFacets) {
        InterfaceFacet* f = interfGeom->GetFacet(sel);
        bool hadAnyTexture = f->sh.countDes || f->sh.countAbs || f->sh.countRefl || f->sh.countTrans || f->sh.countACD || f->sh.countDirection;
        bool hadDirCount = f->sh.countDirection;

        if (enableTexture == 0 || (doRatio && (resolutionU == 0.0 || resolutionV == 0.0))) {
            //Let the user disable textures with the main switch or by typing 0 as resolution
            f->sh.countDes = f->sh.countAbs = f->sh.countRefl = f->sh.countTrans = f->sh.countACD = f->sh.countDirection = false;
        }
        else {
            if (countDesorption < 2) f->sh.countDes = countDesorption;
            if (countAbsorption < 2) f->sh.countAbs = countAbsorption;
            if (countReflection < 2) f->sh.countRefl = countReflection;
            if (countTransparentPass < 2) f->sh.countTrans = countTransparentPass;
            if (angularCoefficient < 2) f->sh.countACD = angularCoefficient;
            if (recordDirectionVectors < 2) f->sh.countDirection = recordDirectionVectors;
        }

        if (useSquareCells == 1 && !IsZero(f->tRatioU - f->tRatioV)) {
            f->tRatioV = f->tRatioU;
        }

        bool hasAnyTexture = f->sh.countDes || f->sh.countAbs || f->sh.countRefl || f->sh.countTrans || f->sh.countACD || f->sh.countDirection;

        //set textures
        try {
            bool needsRemeshing = force || (hadAnyTexture != hasAnyTexture) || (hadDirCount != f->sh.countDirection)
                || (doRatio && ((!IsZero(interfGeom->GetFacet(sel)->tRatioU - resolutionU)) || (!IsZero(interfGeom->GetFacet(sel)->tRatioV - resolutionV))));
            if (needsRemeshing) {
                interfGeom->SetFacetTexture(sel, hasAnyTexture ? (doRatio ? resolutionU : f->tRatioU) : 0.0,
                    hasAnyTexture ? (doRatio ? resolutionV : f->tRatioV) : 0.0,
                    hasAnyTexture ? boundMap : false);
            }
        }
        catch (const std::exception& e) {
            ImIOWrappers::InfoPopup("Error", e.what());
            return;
        }
        catch (...) {
            ImIOWrappers::InfoPopup("Error", "Unexpected error while setting textures");
            return;
        }
        nbPerformed++;
        //prg.SetProgress((double)nbPerformed / (double)selectedFacets.size());
    } //main cycle end
    return;
}

void ImAdvFacetParams::UpdateMemoryEstimate()
{
    if (!interfGeom->IsLoaded()) return;

    if (!enableTexture) {
        memory = FormatMemory(0);
        cells = "0";
        return;
    }

    if (!Util::getNumber(&resolutionU, resolutionUIn) || !Util::getNumber(&resolutionV, resolutionVIn)) {
        memory = "";
        cells = "";
        return;
    }

    size_t ram = 0;
    size_t cell = 0;
    size_t nbFacet = interfGeom->GetNbFacet();
    if (angularCoefficient) {

        for (size_t i = 0; i < nbFacet; i++) {
            InterfaceFacet* f = interfGeom->GetFacet(i);
            //if(f->sp.opacity==1.0) {
            if (f->selected) {
                cell += (size_t)(cellsU * cellsV);
                ram += (size_t)f->GetTexRamSizeForRatio(resolutionU, resolutionV, 1 + mApp->worker.interfaceMomentCache.size());
            }
            else {
                auto nbCells = f->GetNbCell();
                cell += (size_t)(nbCells.first * nbCells.second);
                ram += (size_t)f->GetTexRamSize(1 + mApp->worker.interfaceMomentCache.size());
            }
            //}
        }
        ram += (((cell - 1) * cell) / 2 + 8 * cell) * ((size_t)sizeof(ACFLOAT));

    }
    else {

        for (size_t i = 0; i < nbFacet; i++) {
            InterfaceFacet* f = interfGeom->GetFacet(i);
            if (f->selected) {
                cell += (size_t)(cellsU * cellsV);
                ram += (size_t)f->GetTexRamSizeForRatio(resolutionU, resolutionV, 1 + mApp->worker.interfaceMomentCache.size());
            }
            else {
                auto nbCells = f->GetNbCell();
                cell += (size_t)(nbCells.first * nbCells.second);
                ram += (size_t)f->GetTexRamSize(1 + mApp->worker.interfaceMomentCache.size());
            }
        }

    }

    memory = FormatMemoryLL(ram);
    cells = fmt::format("{}", cell);
}

void ImAdvFacetParams::UpdateCellCount()
{
    if (nbSelected == 1) {
        auto selFacets = interfGeom->GetSelectedFacets();
        Util::getNumber(&resolutionV, resolutionVIn);
        auto nbCells = interfGeom->GetFacet(selFacets.front())->GetNbCellForRatio(resolutionU, resolutionV);
        cellsU = nbCells.first;
        cellsUIn = fmt::format("{}", cellsU);
        cellsV = nbCells.second;
        cellsVIn = fmt::format("{}", cellsV);
    }
    else {
        cellsUIn = "...";
        cellsVIn = "...";
    }
}

void ImAdvFacetParams::EditCellCount()
{
    enableTexture = true;
    // this is the logic from the legacy window, but I'm not sure if it is correct
    // since even if we edit the cellsV it will only update if cellsU contains a valid
    // value
    if (Util::getNumber(&cellsU, cellsUIn) && cellsU != 0) {
        if (useSquareCells == 1 && nbSelected == 1) {
            auto ratio = GetRatioForNbCell(cellsU, cellsU);
            auto selected = interfGeom->GetSelectedFacets();
            InterfaceFacet* fac = interfGeom->GetFacet(selected.front());
            auto nbCells = fac->GetNbCellForRatio(ratio.first, ratio.first);
            cellsV = nbCells.second;
        }
        if (Util::getNumber(&cellsV, cellsVIn) && cellsV != 0) {
            auto ratio = GetRatioForNbCell(cellsU, cellsU);
            resolutionU = ratio.first;
            resolutionUIn = fmt::format("{}", resolutionU);
            resolutionV = ratio.second;
            resolutionVIn = fmt::format("{}", resolutionV);
            if (ratio.first != 0) {
                cellSizeU = 1.0 / ratio.first;
                cellSizeUIn = fmt::format("{}", cellSizeU);
            }
            if (ratio.second != 0) {
                cellSizeV = 1.0 / ratio.second;
                cellSizeVIn = fmt::format("{}", cellSizeV);
            }
            UpdateMemoryEstimate();
        }
    }
}

void ImAdvFacetParams::CalcSojournTime()
{
    if (wallSojourn == 0
        || !(Util::getNumber(&freq, freqIn)
        || !(Util::getNumber(&binding, bindingIn))
        || !(Util::getNumber(&mApp->imWnd->sideBar.fSet.temp, mApp->imWnd->sideBar.fSet.temperatureInput)))) {
        sojournText = ("Wall sojourn time");
        return;
    }
    std::ostringstream tmp;
    tmp << "Wall sojourn time (mean=" << 1.0 / (freq * exp(-binding / (8.31 * mApp->imWnd->sideBar.fSet.temp))) << " s)";
    sojournText = tmp.str();
}

bool ImAdvFacetParams::Apply()
{
    // check inputs before the loop to not have to do it for every selected facet
    bool doSojournFreq = false;
    // slight change from legacy: does not change the freq if wall sojourn is disabled
    if (wallSojourn != 0) {
        if (Util::getNumber(&freq, freqIn)) {
            // is a valid number
            if (freq <= 0.0) {
                ImIOWrappers::InfoPopup("Error", "Wall sojourn time frequency has to be postive");
                return false;
            }
            doSojournFreq = true;
        }
        else if (freqIn != "...") { // is NaN and not "..."
            ImIOWrappers::InfoPopup("Error", "Invalid wall sojourn time frequency");
            return false;
        }
        // is "..." ('do' bool remains false)
    }
    bool doSojournEnergy = false;
    if (wallSojourn != 0) {
        if (Util::getNumber(&binding, bindingIn)) {
            // is a valid number
            if (binding <= 0.0) {
                ImIOWrappers::InfoPopup("Error", "Wall sojourn time second coefficient (Energy) has to be positive");
                return false;
            }
            doSojournEnergy = true;
        }
        else if (bindingIn != "...") { // is NaN and not "..."
            ImIOWrappers::InfoPopup("Error", "Invalid wall sojourn time second coefficient (Energy)");
            return false;
        }
        // is "..." ('do' bool remains false)
    }
    bool doDiffuse = false;
    if (Util::getNumber(&diffuse, diffuseIn)) {
        if (diffuse < 0.0 || diffuse > 1.0) {
            ImIOWrappers::InfoPopup("Error", "Diffuse reflection ratio must be vetween 0 and 1");
            return false; // early return if out of range
        }
        doDiffuse = true; // numer in range
    }
    else if (diffuseIn != "...") {
        // NaN and not "..."
        ImIOWrappers::InfoPopup("Error", "Invalid diffuse reflection ratio");
        return false;
    }
    bool doSpecular = false;
    if (Util::getNumber(&specular, specularIn)) {
        if (specular < 0.0 || specular > 1.0) {
            ImIOWrappers::InfoPopup("Error", "Specular reflection ratio must be vetween 0 and 1");
            return false; // early return if out of range
        }
        doSpecular = true; // numer in range
    }
    else if (specularIn != "...") {
        // NaN and not "..."
        ImIOWrappers::InfoPopup("Error", "Invalid specular reflection ratio");
        return false;
    }
    if (diffuse + specular > 1) {
        ImIOWrappers::InfoPopup("Error", "The sum of diffuse and specular reflection ratios cannot be larger than 1");
        return false;
    }
    bool doReflExp = false;
    if (Util::getNumber(&reflextionExponent, reflextionExponentIn)) {
        if (reflextionExponent <= -1.0) {
            ImIOWrappers::InfoPopup("Error", "Cosine^N exponent must be greater than or equal to -1");
            return false;
        }
        doReflExp = true;
    }
    else if (reflextionExponentIn != "...") {
        ImIOWrappers::InfoPopup("Error", "Invalid cosine^N reflection exponent");
        return false;
    }

    bool doAccomodation = false;
    if (Util::getNumber(&accomodation, accomodationIn)) {
        if (accomodation < 0.0 || accomodation > 1.0) {
            ImIOWrappers::InfoPopup("Error", "Facet accomodation factor must be between 0 and 1");
            return false;
        }
        doAccomodation = true;
    }
    else if (accomodationIn != "...") {
        ImIOWrappers::InfoPopup("Error", "Invalid accomodation factor number");
        return false;
    }
    bool doTeleport = false;
    if (Util::getNumber(&teleport, teleportIn)) {
        if (teleport<-1 || teleport>interfGeom->GetNbFacet()) {
            ImIOWrappers::InfoPopup("Error", "Invalid teleport destination\n(If no teleport: set number to 0)");
            return false;
        }
        else if (teleport > 0 && interfGeom->GetFacet(teleport - 1)->selected) {
            ImIOWrappers::InfoPopup("Error", fmt::format("The teleport destination of facet #{} can't be itself!", teleport));
            return false;
        }
    }
    else if (teleportIn != "...") {
        ImIOWrappers::InfoPopup("Error", "Invalid teleport destination input\n(If no teleport: set number to 0)");
        return false;
    }

    bool structureChanged = false;
    bool doStructure = false;
    // Had to completely rewrite this if from legacy, it is far simpler now
    if (Util::getNumber(&structure, structureIn)) { // input is a valid number
        structure--;
        doStructure = true;
    }
    else if (Contains({ "All", "all" }, structureIn)) { // input is NaN but is the word All
        structure = -1;
        doStructure = true;
    }
    else if (structureIn != "...") { // input is neither a number, 'all' nor '...' so it is invalid
        ImIOWrappers::InfoPopup("Error", "Invalid input in superstructure number");
        return false;
    }
    // it will only get here if structureIn == "..." in which case doStructure is false by default

    bool doLink = false;
    // same deal as above
    if (Util::getNumber(&link, linkIn)) {
        if (structure + 1 == link) {
            ImIOWrappers::InfoPopup("Error", "Link and superstructure can't be the same");
            return false;
        }
        else if (link < 0 || link > interfGeom->GetNbStructure()) {
            ImIOWrappers::InfoPopup("Error", "Link destination points to a structure that doesn't exist");
            return false;
        }
        doLink = true;
    }
    else if (Contains({ "none", "no" }, linkIn)) {
        doLink = true;
        link = 0;
    }
    else if (linkIn != "...") {
        ImIOWrappers::InfoPopup("Error", "Invalid superstructure destination");
        return false;
    }

    LockWrapper lW(mApp->imguiRenderLock);
    if (!mApp->AskToReset(&mApp->worker)) return false;

    auto selectedFacets = interfGeom->GetSelectedFacets();
    for (auto& sel : selectedFacets) {
        InterfaceFacet* f = interfGeom->GetFacet(sel);
        if (drawTexture < 2) f->viewSettings.textureVisible = drawTexture;
        if (drawVolume < 2) f->viewSettings.volumeVisible = drawVolume;

        if (wallSojourn < 2) f->sh.enableSojournTime = wallSojourn;
        if (doSojournFreq) f->sh.sojournFreq = freq;
        if (doSojournEnergy) f->sh.sojournE = binding;

        if (doDiffuse) f->sh.reflection.diffusePart = diffuse;
        if (doSpecular) f->sh.reflection.specularPart = specular;
        if (doReflExp) f->sh.reflection.cosineExponent = reflextionExponent;

        if (doAccomodation) f->sh.accomodationFactor = accomodation;
        if (doTeleport) f->sh.teleportDest = teleport - 1;
        if (doStructure) {
            if (f->sh.superIdx != structure) {
                structureChanged = true;
                f->sh.superIdx = structure;
            }
        }
        if (doLink) {
            f->sh.superDest = link;
            if (link) f->sh.opacity = 1.0;
        }
        if (movingPart < 2) f->sh.isMoving = movingPart;
    }

    //Re-render facets (LockWrapper is alredy set)
    if (structureChanged) interfGeom->BuildGLList();

    ApplyTexture(false);
    return true;
}

// copied from legacy
std::pair<double, double> ImAdvFacetParams::GetRatioForNbCell(size_t nbCellsU, size_t nbCellsV) {

    double ratioU = 0.0;
    double ratioV = 0.0;
    auto selFacets = interfGeom->GetSelectedFacets();

    if (selFacets.size() == 1) {
        for (auto& sel : selFacets) {
            InterfaceFacet* f = interfGeom->GetFacet(sel);
            if (f->selected) {
                double nU = f->sh.U.Norme();
                double nV = f->sh.V.Norme();

                if (nU != 0.0)
                    ratioU = (double)nbCellsU / nU;
                if (nV != 0.0)
                    ratioV = (double)nbCellsV / nV;
            }
        }
    }
    return std::make_pair(ratioU, ratioV);
}