#include "ImguiGeometryViewer.h"
#include "ImguiExtensions.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Interface.h"
#include "GLWindowManager.h"

/*
- GL Viewer is still always behind ImGui content, a rewrite of the viewer drawing might be needed to fix it
	because ImGui background window and foreground drawlists have their own methods of drawing
*/

void ImGeoViewer::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW*140, txtH*40));
	ImGui::Begin("Geometry Viewer", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse);

	availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(2*margin, margin+22));
	availableTLcorner = ImMath::AddVec2(ImGui::GetWindowPos(),ImVec2(margin,0));

	glViewer->SetBounds(static_cast<int>(availableTLcorner.x), static_cast<int>(availableTLcorner.y), static_cast<int>(availableSpace.x), static_cast<int>(availableSpace.y));
	// event passthrough when window is in focus and viewer is hovered
	if (ImGui::IsWindowFocused() && ImGui::IsWindowHovered() && ImMath::IsInsideVec2(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, ImMath::AddVec2(availableSpace, ImVec2(0, 22))), ImGui::GetMousePos())) {
		mApp->imWnd->skipImGuiEvents = true;
	}
	ImGui::GetBackgroundDrawList()->AddRect(ImMath::AddVec2(availableTLcorner, ImVec2(0, 22)), ImMath::AddVec2(availableTLcorner, ImMath::AddVec2(availableSpace, ImVec2(0, 22))), IM_COL32(255, 255, 255, 255));
	ImGui::End();
	glViewer->Paint();
}

void ImGeoViewer::OnShow()
{
	// TODO add separate viewer for this
	glViewer = mApp->viewers[0];
}

void ImGeoViewer::OnHide()
{
	mApp->Place3DViewer();
}
