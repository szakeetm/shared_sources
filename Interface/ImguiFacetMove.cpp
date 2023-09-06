#include "ImguiFacetMove.h"
#include "imgui.h"
#include "ImguiExtensions.h"
#include <string>

void ShowAppFacetMove()
{
    enum movementMode { absolute_offset, direction_and_distance };
    static int mode = absolute_offset;
    static std::string prefix;
    // flags
    ImGui::Begin("Facet Move", NULL, ImGuiWindowFlags_AlwaysAutoResize);
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

    ImGui::Text((prefix + (std::string)"X").c_str()); ImGui::SameLine();
    ImGui::InputFloat("cm##X", &axis_X, 0.01f, 1.0f, "%.3f", 0); //can accept either scientific (%e) or regular float %.3f but not both
    ImGui::Text((prefix + (std::string)"Y").c_str()); ImGui::SameLine();
    ImGui::InputFloat("cm##Y", &axis_Y, 0.01f, 1.0f, "%.3f", 0);
    ImGui::Text((prefix + (std::string)"Z").c_str()); ImGui::SameLine();
    ImGui::InputFloat("cm##Z", &axis_Z, 0.01f, 1.0f, "%.3f", 0);

    ImGui::BeginGroup();
    {
        ImGui::Separator(); ImGui::SameLine();
        ImGui::Text("In direction");

        if (mode == absolute_offset) ImGui::BeginDisabled();

        {
            ImGui::Text("Distance"); ImGui::SameLine();
            ImGui::InputFloat("cm##D", &distance, 0.01f, 1.0f, "%.3f", 0);
        }

        if (mode == absolute_offset) ImGui::EndDisabled();
        int width = ImGui::GetWindowWidth();
        //ImGui::Spacing(width*0.5); ImGui::SameLine();
        if (ImGui::Button("Facet normal"))
        {
            //TODO Facet normal function call
        }

        ImGui::BeginTable("##0", 2);
        {
            ImGui::TableNextColumn();
            ImGui::Separator(); ImGui::SameLine();
            ImGui::Text("Base");
            ImGui::TableSetColumnIndex(0);
            {
                if (ImGui::Button("Selected Vertex"))
                {
                    //TODO Selected Vertex function call
                }
                if (ImGui::Button("Facet center"))
                {
                    //TODO Facet center function call
                }
            }


            if (!base_selected) ImGui::BeginDisabled();

            ImGui::TableNextColumn();
            ImGui::Separator(); ImGui::SameLine();
            ImGui::Text("Direction");
            ImGui::TableSetColumnIndex(1);
            {
                if (!base_selected) {
                    ImGui::Text("Choose base first");
                }
                if (ImGui::Button("Selected Vertex"))
                {
                    //TODO Selected Vertex function call
                }
                if (ImGui::Button("Facet center"))
                {
                    //TODO Facet center function call
                }
            }

            if (!base_selected) ImGui::EndDisabled();
        }
        ImGui::EndTable();

    }
    ImGui::EndGroup();
    ImGui::Separator();
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