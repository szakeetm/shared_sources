#include "ImguiFacetMove.h"
#include "imgui.h"
#include "ImguiExtensions.h"
#include <imgui/imgui_internal.h>
#include "imgui_stdlib/imgui_stdlib.h"
#include <string>

bool baseVertexSelect(InterfaceGeometry* interfGeom);

void ShowAppFacetMove(bool* p_open, MolFlow* mApp, InterfaceGeometry* interfGeom)
{
    mode = absolute_offset;
    ImGui::Begin("Facet Move", p_open, ImGuiWindowFlags_AlwaysAutoResize);

    ImGui::RadioButton("Absolute Offset", &mode, absolute_offset); ImGui::SameLine();
    ImGui::RadioButton("Direction and Distance", &mode, direction_and_distance);

    if (mode == absolute_offset) // Compute labels of input fields based on selected mode
    {
        prefix = "d";
    }
    else
    {
        prefix = "dir";
    }
    
    if (!base_selected)
    {
        selection = "Nothing selected";
    }

    ImGui::Text(prefix+"X"); ImGui::SameLine();
    ImGui::InputText("cm##X", &axis_X);
    ImGui::Text(prefix+"Y"); ImGui::SameLine();
    ImGui::InputText("cm##Y", &axis_Y);
    ImGui::Text(prefix+"Z"); ImGui::SameLine();
    ImGui::InputText("cm##Z", &axis_Z);

    ImGui::BeginGroup();
    {
        ImGui::Separator(); ImGui::SameLine();
        ImGui::Text("In direction");

        if (mode == absolute_offset) ImGui::BeginDisabled();

        {
            ImGui::Text("Distance"); ImGui::SameLine();
            ImGui::InputText("cm##D", &distance);
        }

        if (mode == absolute_offset) ImGui::EndDisabled();
        ImGui::PlaceAtRegionCenter("Facet normal");
        if (ImGui::Button("Facet normal"))
        {
            //TODO Facet normal function call
        }
        ImGuiTableFlags table_flags = ImGuiTableFlags_SizingStretchSame | ImGuiTableFlags_Borders;
        ImGui::BeginTable("options", 2, table_flags);
        {
            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            {
                ImGui::PlaceAtRegionCenter("Base");
                ImGui::Text("Base");
                ImGui::PlaceAtRegionCenter(selection);
                ImGui::Text(selection);
                ImGui::PlaceAtRegionCenter("Selected Vertex");
                if (ImGui::Button("Selected Vertex"))
                {
                    base_selected = baseVertexSelect(interfGeom);
                }
                ImGui::PlaceAtRegionCenter("Facet center");
                if (ImGui::Button("Facet center"))
                {
                    //TODO Facet center function call
                }
            } ImGui::EndGroup();


            if (!base_selected) ImGui::BeginDisabled();

            ImGui::TableNextColumn();
            ImGui::BeginGroup();
            {
                ImGui::PlaceAtRegionCenter("Direction");
                ImGui::Text("Direction");
                if (!base_selected) {
                    ImGui::PlaceAtRegionCenter("Choose base first");
                    ImGui::Text("Choose base first");
                }
                else { // fill the line when message is not to be shown
                    ImGui::Text(" ");
                }
                ImGui::PlaceAtRegionCenter("Selected Vertex");
                if (ImGui::Button("Selected Vertex"))
                {
                    //TODO Selected Vertex function call
                }
                ImGui::PlaceAtRegionCenter("Facet center");
                if (ImGui::Button("Facet center"))
                {
                    //TODO Facet center function call
                }
            }ImGui::EndGroup();

            if (!base_selected) ImGui::EndDisabled();
        }
        ImGui::EndTable();

    }
    ImGui::EndGroup();
    ImGui::Separator();
    ImGui::PlaceAtRegionCenter("Move facets   Copy facets");
    if (ImGui::Button("Move facets"))
    {
        //TODO move facets function call
    } ImGui::SameLine();
    if (ImGui::Button("Copy facets"))
    {
        //TODO copy facets function call
    }

    ImGui::End();
}

bool baseVertexSelect(InterfaceGeometry* interfGeom)
{
    auto selVertices = interfGeom->GetSelectedVertices();
    if (selVertices.size() != 1) { // if condition is not met the porgram crashes with vector subscript out of range, try catch does not catch
        //popup
        base_selected = false;
        return false;
    }
    baseLocation = (Vector3d) * (interfGeom->GetVertex(selVertices[0]));
    selection = fmt::format("Vertex {}", selVertices[0] + 1);
    return true;
}