#include "ImguiPopup.h"

void ImguiPopup::Popup(std::string msg) {
    Popup(msg, "Popup");
}

void ImguiPopup::Popup(std::string msg, std::string title) {
    ImguiPopup::message = msg;
    ImguiPopup::title = title;
    ImguiPopup::window = { ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, ImGui::GetWindowSize().x, ImGui::GetWindowSize().y};
    ImGui::OpenPopup(title.c_str());
}

void ImguiPopup::ShowPopup() {
    ImGuiIO& io = ImGui::GetIO();

    int txtW = ImGui::CalcTextSize(" ").x;
    int txtH = ImGui::GetTextLineHeightWithSpacing();

    float nextX = 10*txtW + ImGui::CalcTextSize(ImguiPopup::message.c_str()).x, nextY = 4 * txtH;
    ImGui::SetNextWindowSize(ImVec2(nextX, nextY));
    ImGui::SetNextWindowPos(ImVec2(ImguiPopup::window.posX + ImguiPopup::window.sizeX / 2 - nextX / 2, ImguiPopup::window.posY + ImguiPopup::window.sizeY / 2 - nextY / 2));
    if (ImGui::BeginPopupModal(title.c_str(), NULL, ImGuiWindowFlags_NoResize))
    {
        ImGui::CaptureKeyboardFromApp(true);
        ImGui::PlaceAtRegionCenter(message);
        ImGui::Text(message);
        ImGui::PlaceAtRegionCenter("OK");
        if (ImGui::Button("OK") || io.KeysDown[ImGui::keyEnter] || io.KeysDown[ImGui::keyEsc] || io.KeysDown[ImGui::keyNumEnter])
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::CaptureKeyboardFromApp(false);
        ImGui::EndPopup();
    }
}

