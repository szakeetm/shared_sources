#include "ImguiVertexMove.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "Interface.h"

void ImVertexMove::Draw() {
	if (!drawn) return;
	float btnWidth = ImGui::CalcTextSize("_Selected Vertex_").x;
	ImGui::SetNextWindowSize(ImVec2(txtW * 40, txtH * 19));
	ImGui::Begin("Move Vertex", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	if (ImGui::RadioButton("Absolute offset", mode == absOffset)) mode = absOffset;
	if (ImGui::RadioButton("Direction and distance", mode == directionDist)) mode = directionDist;
	std::string prefix = mode == absOffset ? "d" : "dir";
	ImGui::TextWithMargin(prefix + "X", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 20);
	ImGui::InputText("cm###xIn", &xIn);

	ImGui::TextWithMargin(prefix + "Y", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 20);
	ImGui::InputText("cm###yIn", &yIn);

	ImGui::TextWithMargin(prefix + "Z", 3 * txtW);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 20);
	ImGui::InputText("cm###zIn", &zIn);

	ImGui::BeginChild("###MVD", ImVec2(0, ImGui::GetContentRegionAvail().y-txtH*1.3),ImGuiChildFlags_Border);

	ImGui::Text("In direction");
	if (mode != directionDist)	ImGui::BeginDisabled();

	ImGui::Text("Distance");
	ImGui::SameLine();
	ImGui::InputText("cm###dIn", &dIn);

	if (mode != directionDist)	ImGui::EndDisabled();
	ImGui::PlaceAtRegionCenter("_Selected Vertex_");
	if (ImGui::Button("Facet normal", ImVec2(btnWidth, 0))) FacetNormalButtonPress();

	if (ImGui::BeginTable("###MVlayoutHelper", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_BordersOuterH)) {
		ImGui::TableNextRow();
		ImGui::TableSetColumnIndex(0);
		ImGui::BeginGroup();

		ImGui::TextCentered("Base");
		ImGui::PlaceAtRegionCenter(baseMsg);
		ImGui::Text(baseMsg);
		ImGui::PlaceAtRegionCenter("_Selected Vertex_");
		if (ImGui::Button("Selected Vertex##B", ImVec2(btnWidth, 0))) BaseSelVertButtonPress();
		ImGui::PlaceAtRegionCenter("_Selected Vertex_");
		if (ImGui::Button("Facet center##B", ImVec2(btnWidth, 0))) BaseFacCentButtonPress();

		ImGui::EndGroup();
		ImGui::TableSetColumnIndex(1);
		ImGui::BeginGroup();

		if (!selectedBase) ImGui::BeginDisabled();
		ImGui::TextCentered("Direction");
		ImGui::PlaceAtRegionCenter(dirMsg);
		ImGui::Text(dirMsg);
		ImGui::PlaceAtRegionCenter("_Selected Vertex_");
		if (ImGui::Button("Selected Vertex##D", ImVec2(btnWidth, 0))) DirSelVertButtonPress();
		ImGui::PlaceAtRegionCenter("_Selected Vertex_");
		if (ImGui::Button("Facet center##D", ImVec2(btnWidth, 0))) DirFacCentButtonPress();
		if (!selectedBase) ImGui::EndDisabled();
		
		ImGui::EndGroup();
		ImGui::EndTable();
	}

	ImGui::EndChild();
	ImGui::Dummy(ImVec2(txtW * 4, 0)); ImGui::SameLine();
	if (ImGui::Button("Move vertices", ImVec2(btnWidth, 0))) ApplyButtonPress(false);
	ImGui::SameLine();
	if (ImGui::Button("Copy vertices", ImVec2(btnWidth, 0))) ApplyButtonPress(true);


	ImGui::End();
}

void ImVertexMove::FacetNormalButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	Vector3d normal = interfGeom->GetFacet(interfGeom->GetSelectedFacets()[0])->sh.N.Normalized();
	xIn = fmt::format("{}", normal.x);
	yIn = fmt::format("{}", normal.y);
	zIn = fmt::format("{}", normal.z);
	mode = directionDist;
}

void ImVertexMove::BaseSelVertButtonPress()
{
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	size_t vId = interfGeom->GetSelectedVertices()[0];
	baseLocation = (Vector3d)*(interfGeom->GetVertex(vId));
	baseMsg = fmt::format("Vertex {}", vId+1);
	selectedBase = true;
}

void ImVertexMove::BaseFacCentButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	size_t fId = interfGeom->GetSelectedFacets()[0];
	baseLocation = interfGeom->GetFacet(fId)->sh.center;
	baseMsg = fmt::format("Center of facet {}", fId + 1);
	selectedBase = true;
	mode = directionDist;
}

void ImVertexMove::DirSelVertButtonPress()
{
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	size_t vId = interfGeom->GetSelectedVertices()[0];
	Vector3d translation = *(interfGeom->GetVertex(vId)) - baseLocation;
	
	xIn = fmt::format("{}", translation.x);
	yIn = fmt::format("{}", translation.y);
	zIn = fmt::format("{}", translation.z);
	dIn = fmt::format("{}", translation.Norme());

	dirMsg = fmt::format("Vertex {}", vId + 1);
}

void ImVertexMove::DirFacCentButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	size_t fId = interfGeom->GetSelectedFacets()[0];
	Vector3d translation = (interfGeom->GetFacet(fId)->sh.center) - baseLocation;

	xIn = fmt::format("{}", translation.x);
	yIn = fmt::format("{}", translation.y);
	zIn = fmt::format("{}", translation.z);
	dIn = fmt::format("{}", translation.Norme());
	
	dirMsg = fmt::format("Center of facet {}", fId + 1);
}

void ImVertexMove::ApplyButtonPress(bool copy)
{
	if (interfGeom->GetNbSelectedVertex() == 0) {
		ImIOWrappers::InfoPopup("Nothing to move", "No vertices selected");
		return;
	}
	if (!Util::getNumber(&x, xIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid X offset/direction");
		return;
	}
	if (!Util::getNumber(&y, yIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid Y offset/direction");
		return;
	}
	if (!Util::getNumber(&z, zIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid Z offset/direction");
		return;
	}
	if (mode == directionDist) {
		if (!Util::getNumber(&d, dIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid offset distance");
			return;
		}
		if (x == y == z == 0.0) {
			ImIOWrappers::InfoPopup("Error", "Direction can't be a null-vector");
			return;
		}
	}
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		interfGeom->MoveSelectedVertex(x, y, z, mode == directionDist, d, copy);
		mApp->worker.MarkToReload();
		mApp->changedSinceSave = true;
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}
