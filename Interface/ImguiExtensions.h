//
// Created by pbahr on 8/2/21.
//

#ifndef MOLFLOW_PROJ_IMGUIEXTENSIONS_H
#define MOLFLOW_PROJ_IMGUIEXTENSIONS_H

#include "imgui/imgui.h"
#include <string>

namespace ImGui {
// Make the UI compact because there are so many fields
    void PushStyleCompact();

    void PopStyleCompact();

// Helper to display a little (?) mark which shows a tooltip when hovered.
// In your own code you may want to display an actual icon if you are using a
// merged icon fonts (see docs/FONTS.md)
    void HelpMarker(const char *desc);

    void PlaceAtWindowCenter(const char *str);

    void PlaceAtRegionCenter(const char *str);

    void PlaceAtRegionRight(const char *str, bool sameLine);

    bool InputRightSide(const char *desc, double *val, const char* format = "%.4f");

// Add spacing of checkbox width
    void AddCheckboxWidthSpacing();

    bool
    BufferingBar(const char *label, float value, const ImVec2 &size_arg, const ImU32 &bg_col, const ImU32 &fg_col);

    bool
    Spinner(const char *label, float radius, int thickness, const ImU32 &color);

    void Loader(float& progress, float& time);
}
#endif //MOLFLOW_PROJ_IMGUIEXTENSIONS_H
