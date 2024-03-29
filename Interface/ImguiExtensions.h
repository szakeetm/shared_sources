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


    void TextCentered(std::string str);
    void PlaceAtRegionCenter(const char *str);
    void PlaceAtRegionCenter(std::string str);

    void PlaceAtRegionRight(const char *str, bool sameLine);

    bool InputDoubleRightSide(const char *desc, double *val, const char* format = "%.4f");
    bool InputTextRightSide(const char* desc, const char* text, ImGuiInputTextFlags flags = 0);
    bool TriState(const char* desc, size_t* v);

// Add spacing of checkbox width
    void AddCheckboxWidthSpacing();

    bool
    BufferingBar(const char *label, float value, const ImVec2 &size_arg, const ImU32 &bg_col, const ImU32 &fg_col);

    bool
    Spinner(const char *label, float radius, int thickness, const ImU32 &color);

    void Loader(float& progress, float& time);
}
#endif //MOLFLOW_PROJ_IMGUIEXTENSIONS_H