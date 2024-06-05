#include "ImguiFacetAlign.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Facet_shared.h"
#include "ImguiPopup.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetAlign::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 20.25f));
	ImGui::Begin("Align selected facets to an other", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
	ImGui::BeginChild("###ASFTAO-1", ImVec2(0, txtH * 4), true);
	ImGui::TextDisabled("Step 1: select facets of the object");
	ImGui::Text(fmt::format("{} facets will be aligned", memorizedSelection.size()));
	if (ImGui::Button("Update from selection")) {
		MemorizeSelection();
	}
	ImGui::EndChild();
	ImGui::BeginChild("###ASFTAO-2", ImVec2(0, txtH * 7), true);
	ImGui::TextDisabled("Step 2: select snapping facets & points");
	ImGui::TextWrapped("1. Choose two facets that will be snapped together.\n\n2. Choose 2-2 vertices on the source and destination \nfacets: One will serve as an anchor point, one as a\ndirection aligner. Once you have 2 facets and 4\nvertices selected, proceed to step 3.");
	ImGui::EndChild();
	ImGui::BeginChild("###ASFTAO-3", ImVec2(0, txtH * 5.5f), true);
	ImGui::TextDisabled("Step 3: align");
	ImGui::Checkbox("Invert normal", &invertNormal);
	ImGui::Checkbox("Swap anchor/direction vertices on source", &swapOnSource);
	ImGui::Checkbox("Swap anchor/direction vertices on destination", &swapOnDestination);
	ImGui::EndChild();
	if (ImGui::Button("Align")) {
		AlignButtonPress(false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Undo")) {
		UndoButtonPress();
	}
	ImGui::End();
}

void ImFacetAlign::MemorizeSelection()
{
	memorizedSelection = interfGeom->GetSelectedFacets();
	oriPositions.clear();
	for (auto& sel : memorizedSelection) {
		std::vector<Vector3d> op;
		for (size_t ind = 0; ind < interfGeom->GetFacet(sel)->sh.nbIndex; ind++)
			op.push_back(*interfGeom->GetVertex(interfGeom->GetFacet(sel)->indices[ind]));
		oriPositions.push_back(op);
	}
}

void ImFacetAlign::AlignButtonPress(bool copy)
{
	if (memorizedSelection.size() == 0) {
		ImIOWrappers::InfoPopup("Nothing to align", "No facets memorized");
		return;
	}
	auto appSelectedFacets = interfGeom->GetSelectedFacets();
	if (appSelectedFacets.size() != 2) {
		ImIOWrappers::InfoPopup("Can't align", "Two facets (source and destination) must be selected");
		return;
	}
	bool foundSource = false;
	size_t sourceFacetId;
	for (auto& mem : memorizedSelection) { //find source facet
		if (Contains(appSelectedFacets, mem)) {
			if (!foundSource) {
				foundSource = true;
				sourceFacetId = mem;
			}
			else {
				ImIOWrappers::InfoPopup("Can't align", "Both selected facets are on the source object. One must be on the destination.");
				return;
			}
		}
	}
	if (!foundSource) {
		ImIOWrappers::InfoPopup("Can't align", "No facet selected on source object.");
		return;
	}
	bool foundDest = false;
	size_t destFacetId;
	for (int i = 0; i < appSelectedFacets.size() && (!foundDest); i++) { //find destination facet
		if (appSelectedFacets[i] != sourceFacetId) {
			destFacetId = appSelectedFacets[i];
			foundDest = true;
		}
	}

	if (!foundDest) {
		ImIOWrappers::InfoPopup("Can't align", "Can't find destination facet");
		return;
	}

	if (interfGeom->GetNbSelectedVertex() != 4) {
		ImIOWrappers::InfoPopup("Can't align", "4 vertices must be selected: two on source and two on destination facets");
		return;
	}

	int anchorSourceVertexId, anchorDestVertexId, dirSourceVertexId, dirDestVertexId;
	anchorSourceVertexId = anchorDestVertexId = dirSourceVertexId = dirDestVertexId = -1;

	//find source anchor and dir vertex
	for (int j = 0; j < interfGeom->GetFacet(sourceFacetId)->sh.nbIndex; j++) {
		if (interfGeom->GetVertex(interfGeom->GetFacet(sourceFacetId)->indices[j])->selected) {
			if (anchorSourceVertexId == -1 && dirSourceVertexId == -1) {
				anchorSourceVertexId = (int)interfGeom->GetFacet(sourceFacetId)->indices[j];
			}
			else if (dirSourceVertexId == -1) {
				dirSourceVertexId = (int)interfGeom->GetFacet(sourceFacetId)->indices[j];
			}
			else {
				ImIOWrappers::InfoPopup("Can't align", "More than two selected vertices are on the source facet. Two must be on the destination.");
				return;
			}
		}
	}

	if (anchorSourceVertexId == -1 || dirSourceVertexId == -1) {
		ImIOWrappers::InfoPopup("Can't align", "Less than two selected vertices found on source facet. Select two (anchor, direction).");
		return;
	}

	//find destination anchor and dir vertex
	for (int j = 0; j < interfGeom->GetFacet(destFacetId)->sh.nbIndex; j++) {
		if (interfGeom->GetVertex(interfGeom->GetFacet(destFacetId)->indices[j])->selected) {
			if (anchorDestVertexId == -1 && dirDestVertexId == -1) {
				anchorDestVertexId = (int)interfGeom->GetFacet(destFacetId)->indices[j];
			}
			else if (dirDestVertexId == -1) {
				dirDestVertexId = (int)interfGeom->GetFacet(destFacetId)->indices[j];
			}
			else {
				ImIOWrappers::InfoPopup("Can't align", "More than two selected vertices are on the destination facet. Two must be on the source.");
				return;
			}
		}
	}
	if (anchorDestVertexId == -1 || dirDestVertexId == -1) {
		ImIOWrappers::InfoPopup("Can't align", "Less than two selected vertices found on destination facet. Select two (anchor, direction).");
		return;
	}
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		interfGeom->AlignFacets(memorizedSelection, sourceFacetId, destFacetId, anchorSourceVertexId, anchorDestVertexId, dirSourceVertexId, dirDestVertexId, invertNormal, swapOnSource, swapOnDestination, copy, &mApp->worker);
#if defined(MOLFLOW)
		if (copy) mApp->worker.GetGeometry();
#endif
		mApp->worker.MarkToReload();

		mApp->changedSinceSave = true;
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void ImFacetAlign::UndoButtonPress()
{
	LockWrapper lW(mApp->imguiRenderLock);
	if (!mApp->AskToReset(&mApp->worker)) return;
	for (size_t i = 0; i < memorizedSelection.size(); i++) {
		InterfaceFacet* f = interfGeom->GetFacet(memorizedSelection[i]);
		for (size_t j = 0; j < f->sh.nbIndex; j++) {
			interfGeom->GetVertex(f->indices[j])->SetLocation(this->oriPositions[i][j]);
		}
	}
	interfGeom->InitializeGeometry();
	mApp->worker.MarkToReload();
	mApp->UpdateFacetlistSelected();
	mApp->UpdateViewers();
}
