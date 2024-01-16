#include "ImguiFacetMirrorProject.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImFacetMirrorProject::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 16));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Mirror/project selected facets", &drawn, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("###FPDM",ImVec2(0, ImGui::GetContentRegionAvail().y - 3 * txtH), true);
	ImGui::TextDisabled("Plane definition mode");
	if (ImGui::RadioButton("XY plane", mode == xy)) mode = xy;
	if (ImGui::RadioButton("YZ plane", mode == yz)) mode = yz;
	if (ImGui::RadioButton("XZ plane", mode == xz)) mode = xz;

	if (ImGui::RadioButton("Plane of facet #", mode == planeOfFacet)) mode = planeOfFacet;
	ImGui::SameLine();
	if (mode != planeOfFacet) ImGui::BeginDisabled();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMfacet", &facetIdInput); ImGui::SameLine();
	if (ImGui::Button("<-Get selected")) {

	}
	if (mode != planeOfFacet) ImGui::EndDisabled();
	if(ImGui::RadioButton("Define by 3 selected vertices", mode == byVerts)) mode = byVerts;
	if(ImGui::RadioButton("Define by plane equation", mode == byEqation)) mode = byEqation;
	if (mode != byEqation) ImGui::BeginDisabled();

	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMX", &xFactorInput); ImGui::SameLine(); ImGui::Text("*X+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMY", &yFactorInput); ImGui::SameLine(); ImGui::Text("*Y+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMZ", &zFactorInput); ImGui::SameLine(); ImGui::Text("*Z+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMW", &offsetInput); ImGui::SameLine(); ImGui::Text("=0");

	if (mode != byEqation) ImGui::EndDisabled();

	ImGui::EndChild();
	if (ImGui::Button("Mirror facet")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Copy mirror facet")) {

	}
	if (ImGui::Button("Project facet")) {

	}ImGui::SameLine();
	if (ImGui::Button("Copy project facet")) {

	}ImGui::SameLine();
	if (enableUndo) ImGui::BeginDisabled();
	if (ImGui::Button("Undo projection")) {

	}
	if (enableUndo) ImGui::EndDisabled();
	ImGui::End();
}
