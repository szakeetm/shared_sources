#include "ImguiGeometryViewer.h"
#include "ImguiExtensions.h"
#include "Interface.h"

void ImGeoViewer::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW*140, txtH*40));
	ImGui::Begin("Geometry Viewer", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBackground);
	availableSpace = ImMath::SubstractVec2(ImGui::GetWindowSize(), ImVec2(0, 0));
	availableTLcorner = ImGui::GetWindowPos();
	glViewer->SetPosition(availableTLcorner.x, availableTLcorner.y);
	glViewer->SetSize(availableSpace.x, availableSpace.y);
	glViewer->SetBounds(availableTLcorner.x, availableTLcorner.y, availableSpace.x, availableSpace.y);
	glViewer->Paint(); // Dont know why it's not showing
	ImGui::End();
}

void ImGeoViewer::OnShow()
{
	if (glViewer == nullptr) {
		glViewer = new GeometryViewer(MAX_VIEWER - 1);
		mApp->viewers[MAX_VIEWER - 1] = glViewer; // perhaps set MAX_VIEWER to 5 in the future
		glViewer->SetWorker(&mApp->worker);
	}
	glViewer->SetEnabled(true);
	glViewer->SetVisible(true);
	glViewer->SetFocus(true);
}
