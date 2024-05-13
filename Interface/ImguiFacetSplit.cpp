#include "ImguiFacetSplit.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Helper/StringHelper.h"
#include "Interface.h"
#include "ImguiWindow.h"

void ImFacetSplit::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 11.3));
	ImGui::Begin("Split Facet", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	ImGui::BeginChild("###FSPDM", ImVec2(0, ImGui::GetContentRegionAvail().y - txtH * 1.3), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Plane definition mode");
	if (ImGui::RadioButton("By equation", mode == Mode::equation)) mode = Mode::equation;
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqX", &xIn);
	ImGui::AlignTextToFramePadding();
	ImGui::SameLine();
	ImGui::Text("*X +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqY", &yIn);
	ImGui::SameLine();
	ImGui::Text("*Y +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqZ", &zIn);
	ImGui::SameLine();
	ImGui::Text("*Z +");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###eqW", &wIn);
	ImGui::SameLine();
	ImGui::Text("= 0");
	if(ImGui::Button("XY plane")) {
		mode = Mode::equation;
		xIn = "0";
		yIn = "0";
		zIn = "1";
		wIn = "0";
	} ImGui::SameLine();
	if(ImGui::Button("YZ plane")) {
		mode = Mode::equation;
		xIn = "1";
		yIn = "0";
		zIn = "0";
		wIn = "0";
	} ImGui::SameLine();
	if(ImGui::Button("XZ plane")) {
		mode = Mode::equation;
		xIn = "0";
		yIn = "1";
		zIn = "0";
		wIn = "0";
	}
	if (ImGui::RadioButton("Plane of facet", mode == Mode::facet)) mode = Mode::facet;
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW*6);
	ImGui::InputText("###fId", &fIdIn);
	ImGui::SameLine();
	if (ImGui::Button("<- Get selected")) {
		if (interfGeom->GetNbSelectedFacets() != 1) {
			ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		}
		else
		{
			facetId = interfGeom->GetSelectedFacets()[0];
			fIdIn = fmt::format("{}", facetId+1);
		}
		mode = Mode::facet;
	}
	if (ImGui::RadioButton("By 3 selected vertices", mode == Mode::vertices)) mode = Mode::vertices;
	ImGui::EndChild();
	if (ImGui::Button("Split")) {
		SplitButtonPress();
	} ImGui::SameLine();
	bool disable = !enableUndo;
	if (disable) ImGui::BeginDisabled();
	if (ImGui::Button("Undo")) {
		UndoButtonPress();
	}
	if (disable) ImGui::EndDisabled();
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text(output);
	ImGui::End();
}

void ImFacetSplit::Reset()
{
	for (auto facet : deletedFacetList) {
		delete facet.f;
	}
	deletedFacetList.clear();
	enableUndo = false;
	output = "";
}

void ImFacetSplit::SplitButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() == 0) {
		ImIOWrappers::InfoPopup("Nothing to split", "No facets selected");
		return;
	}
	
	Vector3d P0, N;
	double nN2;

	switch (mode) {
	case none:
		ImIOWrappers::InfoPopup("Error", "Select plane definition mode");
		break;
	case equation:
		if (!Util::getNumber(&x, xIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid A coefficient");
			return;
		}
		if (!Util::getNumber(&y, yIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid B coefficient");
			return;
		}
		if (!Util::getNumber(&z, zIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid C coefficient");
			return;
		}
		if (!Util::getNumber(&w, wIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid D coefficient");
			return;
		}
		if (x == y == z == 0) {
			ImIOWrappers::InfoPopup("Error", "A B and C are all 0, this is not a valid plane definition");
			return;
		}
		N.x = x; N.y = y; N.z = z;
		P0.x = 0.0; P0.y = 0; P0.z = 0;
		if (x != 0) P0.x = -w / x;
		else if (y != 0) P0.y = -w / y;
		else if (z != 0) P0.z = -w / z;
		break;
	case facet:
		if (!Util::getNumber(&facetId, fIdIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid number in facet ID");
			return;
		}
		if (facetId > interfGeom->GetNbFacet() || facetId < 1) {
			ImIOWrappers::InfoPopup("Error", "No such facet");
			return;
		}
		P0 = *interfGeom->GetVertex(interfGeom->GetFacet(facetId - 1)->indices[0]);
		N = interfGeom->GetFacet(facetId - 1)->sh.N;
		break;
	case vertices:
		if (interfGeom->GetNbSelectedVertex() != 3) {
			ImIOWrappers::InfoPopup("Error", "Select exactly 3 vertices");
			return;
		}
		{
			auto selectedVertexIds = interfGeom->GetSelectedVertices();
			Vector3d U2 = (*interfGeom->GetVertex(selectedVertexIds[0]) - *interfGeom->GetVertex(selectedVertexIds[1])).Normalized();
			Vector3d V2 = (*interfGeom->GetVertex(selectedVertexIds[0]) - *interfGeom->GetVertex(selectedVertexIds[2])).Normalized();
			Vector3d N2 = CrossProduct(V2, U2); //We have a normal vector
			nN2 = N2.Norme();
			if (nN2 < 1e-8) {
				ImIOWrappers::InfoPopup("Can't define plane", "The selected 3 vertices are on a line.");
				return;
			}
			// Normalize N2
			N = N2 * (1.0 / nN2);
			P0 = *(interfGeom->GetVertex(selectedVertexIds[0]));
		}
		break;
	default:
		break;
	}
	// perform split
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		GLProgress_GUI prg = GLProgress_GUI("Splitting facets", "Facet split");

		for (auto facet : deletedFacetList) {
			delete facet.f;
		}
		deletedFacetList.clear();
		nbCreated = 0;
		deletedFacetList = interfGeom->SplitSelectedFacets(P0, N, &nbCreated, prg);
		nbFacet = interfGeom->GetNbFacet();
		output = fmt::format("{} facet split, creating {} new", deletedFacetList.size(), nbCreated);
		if (deletedFacetList.size() > 0) enableUndo = true;
		mApp->worker.MarkToReload();
		mApp->UpdateModelParams();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void ImFacetSplit::DoUndo() {
	LockWrapper lW(mApp->imguiRenderLock);
	deletedFacetList.clear();
	enableUndo = false;
	output = "";
	//Renumberformula
	mApp->worker.MarkToReload();
	mApp->UpdateModelParams();
	mApp->UpdateFacetlistSelected();
	mApp->UpdateViewers();
}

void ImFacetSplit::UndoButtonPress()
{
	if (nbFacet == interfGeom->GetNbFacet()) { //Assume no change since the split operation
		std::vector<size_t> newlyCreatedList;
		for (size_t index = (interfGeom->GetNbFacet() - nbCreated); index < interfGeom->GetNbFacet(); index++) {
			newlyCreatedList.push_back(index);
		}
		interfGeom->RemoveFacets(newlyCreatedList);
		LockWrapper lW(mApp->imguiRenderLock);
		interfGeom->RestoreFacets(deletedFacetList, false); //Restore to original position
		DoUndo();
	}
	else {
		auto f = [this]() {
			LockWrapper lW(mApp->imguiRenderLock);
			interfGeom->RestoreFacets(deletedFacetList, true); //Restore to end
			DoUndo();
			};
		mApp->imWnd->popup.Open("Undo split", "Geometry changed since split, restore to end without deleting the newly created facets?",
			{ std::make_shared<ImIOWrappers::ImButtonFunc>("Ok", f, ImGuiKey_Enter, ImGuiKey_KeypadEnter),
				std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", 0, ImGuiKey_Escape) });
	}
}
