// dear imgui: wrappers for C++ standard library (STL) types (std::string, etc.)
// This is also an example of how you may wrap your own similar types.

// Changelog:
// - v0.10: Initial version. Added InputText() / InputTextMultiline() calls with std::string

// See more C++ related extension (fmt, RAII, syntaxis sugar) on Wiki:
//   https://github.com/ocornut/imgui/wiki/Useful-Extensions#cness

#pragma once
#include "imgui.h"
#include "imgui_internal.h"
#include <string>

namespace ImGui
{
    // ImGui::InputText() with std::string
    // Because text input needs dynamic resizing, we need to setup a callback to grow the capacity
    IMGUI_API bool  InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool  InputText(std::string label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool  InputTextMultiline(const char* label, std::string* str, const ImVec2& size = ImVec2(0, 0), ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API bool  InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* user_data = nullptr);
    IMGUI_API void  Text(std::string str);
    IMGUI_API void  TextWrapped(std::string str);
    IMGUI_API bool  MenuItem(std::string label, std::string shortcut);
    IMGUI_API bool  MenuItem(std::string label);
    IMGUI_API bool  Selectable(std::string label, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0));
    IMGUI_API bool  BeginCombo(std::string label, std::string preview_value, ImGuiComboFlags flags = 0);
}
