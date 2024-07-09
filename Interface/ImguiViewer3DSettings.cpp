#include "ImguiViewer3DSettings.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImViewerSettings::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW*35, txtH*30), ImGuiCond_FirstUseEver);
	if (positionChangePending) {
		ImGui::SetNextWindowPos(newPos);
		positionChangePending = false;
	}
	ImGui::Begin("3D Viewer Settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::TextDisabled(title.c_str()); // TODO: make dynamic based on selected viewer

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
	ImGui::Checkbox(u8"Hide Normals, u\u20D7, v\u20D7 vectors, indices", &supressUI);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("when more than"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	if (supressUI) ImGui::BeginDisabled();
	ImGui::InputText("##hideLimitIn", &supressUILimitIn); ImGui::SameLine();
	if (supressUI) ImGui::EndDisabled();
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

void ImViewerSettings::SetPos(ImVec2 pos)
{
	newPos = pos;
	positionChangePending = true;
}

void ImViewerSettings::CrossSectionButtonPress()
{
	// TODO: open ImGUi CrossSection tool
}

void ImViewerSettings::ApplyButtonPress()
{

}

void ImViewerSettings::Update()
{
	viewer = mApp->viewers[mApp->curViewer];
	fMode = (ShowFacetMode)viewer->volumeRenderMode;
	showHiddenEdges = viewer->showHiddenFacet;
	showHiddenVertex = viewer->showHiddenVertex;
	showTextureGrid = viewer->showMesh;
	showTimeOverlay = viewer->showTime;

	largerHitDot = viewer->bigDots;
	showDirection = viewer->showDir;
	showTeleports = viewer->showTP;

	transitionStep = viewer->transStep;
	transitionStepIn = fmt::format("{}", transitionStep);

	angleStep = viewer->angleStep;
	angleStepIn = fmt::format("{}", angleStep);

	numOfLines = viewer->dispNumHits;
	numOfLinesIn = fmt::format("{}", numOfLines);

	numOfLeaks = viewer->dispNumLeaks;
	numOfLeaksIn = fmt::format("{}", numOfLeaks);

	title = fmt::format("Viewer#{}", mApp->curViewer + 1);
	
	normeRatio = interfGeom->GetNormeRatio();
	normeRatioIn = fmt::format("{}", normeRatio);
	normalize = interfGeom->GetAutoNorme();
	center = interfGeom->GetCenterNorme();

	supressUI = viewer->hideLot != -1;
	supressUILimit = viewer->hideLot;
	if (supressUI) supressUILimitIn = fmt::format("{}", supressUILimit);
	else supressUILimitIn = "";
	
}

void ImViewerSettings::OnShow()
{
	Update();
}
