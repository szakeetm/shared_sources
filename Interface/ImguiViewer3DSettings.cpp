#include "ImguiViewer3DSettings.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"

void ImViewerSettings::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW*33, txtH*29), ImGuiCond_FirstUseEver);
	if (positionChangePending) {
		ImGui::SetNextWindowPos(newPos);
		positionChangePending = false;
	}
	ImGui::Begin("3D Viewer Settings", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::TextDisabled(title.c_str()); // TODO: make dynamic based on selected viewer

	ImGui::TextWithMargin("Show facet", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
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
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##TranslationStepIn", &translationStepIn);

	ImGui::TextWithMargin("Angle step", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##AngleStepIn", &angleStepIn);

	ImGui::TextWithMargin("Number of lines", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##numberOfLinesIn", &numOfLinesIn);

	ImGui::TextWithMargin("Number of leaks", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
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
	if (!supressUI) ImGui::BeginDisabled();
	ImGui::InputText("##hideLimitIn", &supressUILimitIn); ImGui::SameLine();
	if (!supressUI) ImGui::EndDisabled();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("facets sel.");

	if (ImGui::Button("Cross section")) CrossSectionButtonPress();

	ImGui::Separator();
	ImGui::TextDisabled("Direction field");
	ImGui::Checkbox("Show direction (selected viewer only)", &showDirection);
	ImGui::AlignTextToFramePadding();
	ImGui::Text("Set for all viewers:");
	ImGui::TextWithMargin("Norme ratio", txtW * 14); ImGui::SameLine();
	ImGui::SetNextItemWidth(-1);
	ImGui::InputText("##normeRatioIn", &normeRatioIn);
	ImGui::Checkbox("Normalize", &normalize); ImGui::SameLine();
	ImGui::Checkbox("Center", & center);

	ImGui::Separator();

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
	// validate user input
	if(!Util::getNumber(&translationStep, translationStepIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid translation step value");
		return;
	}
	if (!Util::getNumber(&angleStep, angleStepIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid angle step value");
		return;
	}
	if (!Util::getNumber(&numOfLines, numOfLinesIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid number of displayed hits value");
		return;
	}
	else if (numOfLines < 1 || numOfLines > 2048) {
		ImIOWrappers::InfoPopup("Error", "Invalid number of displayed hits.\nMust be between 1 and 2048");
		return;
	}
	if (!Util::getNumber(&numOfLeaks, numOfLeaksIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid number of displayed leaks value");
		return;
	}
	else if (numOfLeaks < 1 || numOfLeaks > 2048) {
		ImIOWrappers::InfoPopup("Error", "Invalid number of leaks hits.\nMust be between 1 and 2048");
		return;
	}
	if (supressUI) {
		if (!Util::getNumber(&supressUILimit, supressUILimitIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid number of selected facets value");
			return;
		}
		else if (supressUILimit < 2) {
			ImIOWrappers::InfoPopup("Error", "Invalid number of selected facets.\nMust be larger than 2");
			return;
		}
	}
	if (!Util::getNumber(&normeRatio, normeRatioIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid norme ratio value");
		return;
	}
	// assign values
	viewer->volumeRenderMode = static_cast<VolumeRenderMode>(fMode);
	viewer->transStep = translationStep;
	viewer->angleStep = angleStep;
	viewer->dispNumHits = numOfLines;
	viewer->dispNumLeaks = numOfLeaks;
	viewer->showHiddenFacet = showHiddenEdges;
	viewer->showHiddenVertex = showHiddenVertex;
	viewer->showMesh = showTextureGrid;
	viewer->bigDots = largerHitDot;
	viewer->showDir = showDirection;
	viewer->showTime = showTimeOverlay;
	viewer->showTP = showTeleports;

	interfGeom->SetNormeRatio(normeRatio);
	interfGeom->SetAutoNorme(normalize);
	interfGeom->SetCenterNorme(center);

	viewer->hideLot = supressUI ? supressUILimit : -1;

	bool neededMesh = mApp->needsMesh;
	mApp->CheckNeedsTexture();
	bool needsMesh = mApp->needsMesh;
	if (!needsMesh && neededMesh) { //We just disabled mesh
		interfGeom->ClearFacetMeshLists();
	}
	else if (needsMesh && !neededMesh) { //We just enabled mesh
		interfGeom->BuildFacetMeshLists();
	}
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

	translationStep = viewer->transStep;
	translationStepIn = fmt::format("{}", translationStep);

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
