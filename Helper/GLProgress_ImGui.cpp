#include "GLProgress_ImGui.h"
#include "../imgui/imgui.h"
#include "ImguiWindow.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
extern MolFlow* mApp;
#endif
extern char fileSaveFilters[];
#if defined(SYNRAD)
extern SynRad* mApp;
#include "../src/SynRad.h"
#endif

void ImProgress::Draw() {
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
	if (title == "") title = "Progress##0";
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	if(ImGui::Begin(this->title.c_str(), NULL, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoMove)) {
		ImGui::TextWrapped(this->status.c_str());
		ImGui::ProgressBar((static_cast<float>(progress) / 100.0f));
		ImGui::End();
	}
}

void ImProgress::Hide()
{
	drawn = false;
}

void ImProgress::SetProgress(const double prg) {
	int newPrg = static_cast<int>(prg * 100.0);
	if (progress != newPrg) {
		progress = newPrg;
	}
}

void ImProgress::SetMessage(const std::string& msg, const bool newLine, const bool forceDraw) {
	std::string newStatus;
	if (newLine) {
		newStatus = status + "\n" + msg;
	}
	else {
		newStatus = msg;
	}

	if (forceDraw || newStatus!=status) {
		status = newStatus;
	}
}

void ImProgress::SetTitle(std::string title)
{
	this->title = title;
}

void ImProgress::SetVisible(bool value) {
	drawn = value;
}