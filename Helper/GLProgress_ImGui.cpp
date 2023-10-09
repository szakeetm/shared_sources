#include "GLProgress_ImGui.h"
#include "../imgui/imgui.h"

void MyProgress::Draw() {
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(ImGui::CalcTextSize(" ").x * 70, 0));
	if (title == "") title = "Progress##0";
	if(ImGui::Begin(this->title.c_str(), NULL, ImGuiWindowFlags_NoCollapse)) {
		ImGui::TextWrapped(this->status.c_str());
		ImGui::ProgressBar((static_cast<float>(progress) / 100.0));
		ImGui::End();
	}
}
void MyProgress::Show() {
	drawn = true;
	ImGuiIO& io = ImGui::GetIO();
	ImGui::SetNextWindowPos(ImVec2(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f), ImGuiCond_Always, ImVec2(0.5f, 0.5f));
	Draw();
};

void MyProgress::Hide() { drawn = false; };

void MyProgress::SetProgress(const double prg) {
	int newPrg = static_cast<int>(prg * 100.0);
	progress = newPrg;
}

void MyProgress::SetTitle(std::string title)
{
	this->title = title;
}

void MyProgress::Toggle() {
	if (drawn) Hide();
	else Show();
};

void MyProgress::SetVisible(bool value) {
	Show();
}