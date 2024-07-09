#include "ImguiViewer3DSettings.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImViewerSettings::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW*35, txtH*30));
	ImGui::Begin("3D Viewer Settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::TextDisabled("Viewer#1"); // TODO: make dynamic based on selected viewer

	ImGui::TextWithMargin("Show facet", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	if (ImGui::BeginCombo("##ShowFacetCombo", comboText)) {
		if (ImGui::Selectable("Front & Back")) {
			comboText = "Front & Back";
			fMode = front_back;
		}
		if (ImGui::Selectable("Front only")) {
			comboText = "Front onky";
			fMode = front;
		}
		if (ImGui::Selectable("Back only")) {
			comboText = "Back only";
			fMode = back;
		}
		ImGui::EndCombo();
	}

	ImGui::TextWithMargin("Translation step", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	ImGui::InputText("##TranslationStepIn", &transitionStepIn);

	ImGui::TextWithMargin("Angle step", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	ImGui::InputText("##AngleStepIn", &angleStepIn);

	ImGui::TextWithMargin("Number of lines", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	ImGui::InputText("##numberOfLinesIn", &numOfLinesIn);

	ImGui::TextWithMargin("Number of leaks", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	ImGui::InputText("##numberofLeaksIn", &numOfLeaksIn);

	ImGui::Checkbox("Show hidden edges (selected facets)", &showHiddenEdges);
	ImGui::Checkbox("Show hidden vertex (if selected)", &showHiddenVertex);
	ImGui::Checkbox("Show texture grid", &showTextureGrid);
	ImGui::Checkbox("Large dots for hits", &largerHitDot);
	ImGui::Checkbox("Show Teleports", &showTeleports);
	ImGui::Checkbox("Show time overlay", &showTimeOverlay);
	ImGui::Checkbox(u8"Hide Normals, u\u20D7, v\u20D7 vectors, indices", &hideUIByLimit);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("when more than"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("##hideLimitIn", &hideUILimitIn); ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("facets sel.");

	if (ImGui::Button("Cross section")) CrossSectionButtonPress();

	ImGui::TextDisabled("Direction field");
	ImGui::Checkbox("Show direction (selected viewer only)", &showDirection);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Set for all viewers:");
	ImGui::TextWithMargin("Norme ratio", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 14);
	ImGui::InputText("##normeRatioIn", &normeRatioIn);
	ImGui::Checkbox("Normalize", &normalize); ImGui::SameLine();
	ImGui::Checkbox("Center", & center);

	ImGui::Dummy(ImVec2(txtW, txtH));

	if (ImGui::Button("Apply")) ApplyButtonPress();

	ImGui::End();
}

void ImViewerSettings::CrossSectionButtonPress()
{
}

void ImViewerSettings::ApplyButtonPress()
{
}
