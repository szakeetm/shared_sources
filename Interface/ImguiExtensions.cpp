//
// Created by pbahr on 8/2/21.
//

#ifndef MOLFLOW_PROJ_IMGUIEXTENSIONS_H
#define MOLFLOW_PROJ_IMGUIEXTENSIONS_H

#include "imgui/imgui.h"
#include <string>

// Make the UI compact because there are so many fields
void PushStyleCompact() {
    ImGuiStyle &style = ImGui::GetStyle();
    ImGui::PushStyleVar(
            ImGuiStyleVar_FramePadding,
            ImVec2(style.FramePadding.x, (float) (int) (style.FramePadding.y * 0.60f)));
    ImGui::PushStyleVar(
            ImGuiStyleVar_ItemSpacing,
            ImVec2(style.ItemSpacing.x, (float) (int) (style.ItemSpacing.y * 0.60f)));
}

void PopStyleCompact() { ImGui::PopStyleVar(2); }

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
void HelpMarker(const char *desc) {
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered()) {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void PlaceAtWindowCenter(const char *str) {
    const std::string btnText = str;
    float font_size = ImGui::GetFontSize() * btnText.size() / 2;
    ImGui::NewLine();
    ImGui::SameLine((ImGui::GetWindowSize().x / 2) - font_size + (font_size / 2));
}

void PlaceAtRegionCenter(const char *str) {
    // float font_size = ImGui::GetFontSize() * btnText.size() / 2;
    float font_size = ImGui::CalcTextSize(str).x;
    ImGui::NewLine();
    ImGui::SameLine((ImGui::GetContentRegionAvail().x * 0.5f) - (font_size / 2));
}

void PlaceAtRegionRight(const char *str, bool sameLine) {
    // float font_size = ImGui::GetFontSize() * btnText.size() / 2;
    float font_size = ImGui::CalcTextSize(str).x;
    if (!sameLine)
        ImGui::NewLine();
    ImGui::SameLine(ImGui::GetContentRegionAvail().x -
    (font_size + ImGui::GetStyle().FramePadding.x * 2));
}

bool InputRightSide(const char *desc, double *val, const char *format) {
    double tmp = *val;
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

    return *val != tmp; // true if changed
}

// Add spacing of checkbox width
void AddCheckboxWidthSpacing() {
    ImGui::NewLine();
    ImGui::SameLine(ImGui::GetFrameHeightWithSpacing() +
    ImGui::GetStyle().FramePadding.y * 4);
}

#endif //MOLFLOW_PROJ_IMGUIEXTENSIONS_H
