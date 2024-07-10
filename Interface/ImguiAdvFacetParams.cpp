#include "ImguiAdvFacetParams.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"

void ImAdvFacetParams::Draw()
{
    if (!drawn) return;
    ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 55, txtH * 10), ImVec2(txtW * 500, txtH * 100));
	ImGui::Begin("Advanced Facet Parameters", &drawn, ImGuiWindowFlags_NoSavedSettings);
    if (ImGui::CollapsingHeader("Texture properties"))
    {
        ImGui::Checkbox("Enable texture", &enableTexture);
        ImGui::SameLine();
        ImGui::Checkbox("Use square cells", &useSquareCells);
        ImGui::SameLine();
        if (ImGui::Button("Force remesh")) {}

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Resolution:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##ResolutionA", &resolutionInA);
        ImGui::SameLine();
        ImGui::Text("cells/cm");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##ResolutionB", &resolutionInB);
        ImGui::SameLine();
        ImGui::Text("cm/cell");

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Number of cells:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##CellsX", &cellsXIn);
        ImGui::SameLine();
        ImGui::Text("x");
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::SameLine();
        ImGui::InputText("##CellsY", &cellsYIn);

        if (ImGui::BeginTable("##AFPLayoutHelper", 2)) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Checkbox("Count desorption", &countDesorption);
            ImGui::Checkbox("Count absorption", &countAbsorption);
            ImGui::Checkbox("Angular coefficient", &angularCoefficient);
            ImGui::TableNextColumn();
            ImGui::Checkbox("Count reflection", &countReflection);
            ImGui::Checkbox("Count transparent pass", &countTransparentPass);
            ImGui::Checkbox("Record direction vectors", &recordDirectionVectors);
            ImGui::EndTable();
        }

    }

    if (ImGui::CollapsingHeader("Texture cell / memory"))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Memory:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##Memory", &memoryIn);
        ImGui::SameLine();
        ImGui::Text("bytes\tCells:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##cells", &cellsIn);
    }

    if (ImGui::CollapsingHeader("Additional parameters"))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Reflection:", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##diffuseIn", &diffuseIn);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::TextWithMargin("part diffuse.", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##specularIn", &specularIn);
        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin(" \t", txtW * 10);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##cosineIn", &cosineIn);
        ImGui::SameLine();
        ImGui::TextWithMargin("part cosine^", txtW * 10);
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::SameLine();
        ImGui::InputText("##cosineNIn", &cosineNIn);

        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Accomodation coefficient:", txtW * 20);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##Accommodation", &accommodationIn);


        ImGui::AlignTextToFramePadding();
        ImGui::TextWithMargin("Teleport to facet:", txtW * 20);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(-1);
        ImGui::InputText("##TeleportTo", &teleportIn);

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Structure:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        ImGui::InputText("##Structure", &structureIn);
        ImGui::SameLine();
        ImGui::Text("Link to:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 10);
        ImGui::InputText("##LinkTo", &linkIn);

        ImGui::Checkbox("Moving part", &movingPart);
        ImGui::Checkbox("Wall sojourn time", &wallSojourn);
        ImGui::SameLine();
        ImGui::HelpMarker("Info");

        ImGui::AlignTextToFramePadding();
        ImGui::Text("Attempt freq:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##AttemptFreq", &freqIn);
        ImGui::SameLine();
        ImGui::Text("Hz\t Binding E:");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(txtW * 6);
        ImGui::InputText("##BindingE", &bindingIn);
        ImGui::SameLine();
        ImGui::Text("J/mole");
    }

    if (ImGui::CollapsingHeader("View settings"))
    {
        ImGui::Checkbox("Draw Texture", &drawTexture);
        ImGui::SameLine();
        ImGui::Checkbox("Draw Volume", &drawVolume);
        ImGui::SameLine();
        if (ImGui::Button("<- Change draw")) {}
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

        if (ImGui::Button("Copy")) {}
        ImGui::SameLine();
        if (ImGui::Button("Export to CSV")) {}
        ImGui::SameLine();
        if (ImGui::Button("Import CSV")) {}
        ImGui::SameLine();
        if (ImGui::Button("Release recorded")) {}
    }
	ImGui::End();
}
