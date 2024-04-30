#include "ImguiCollapseSettings.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "ImguiPopup.h"
#include "Interface.h"
#include "ImguiWindow.h"

// legacy windows to be updated
#include "VertexCoordinates.h"
#include "FacetCoordinates.h"
#include "../../src/Interface/ProfilePlotter.h"
#include "HistogramPlotter.h"
#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#include "../../src/Interface/TimewisePlotter.h"
#include "../../src/Interface/PressureEvolution.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#include "../src/Interface/SpectrumPlotter.h"
#endif

void ImCollapse::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW*40, txtH*13));
	ImGui::Begin("Collapse Settings", &drawn, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);

	ImGui::Checkbox("Vertices closer than (cm):", &enableVertDistLimit);
	if (!enableVertDistLimit) ImGui::BeginDisabled();
	ImGui::SameLine();
	float dummyWidth = ImGui::GetContentRegionAvail().x - txtW * 9;
	ImGui::Dummy(ImVec2(dummyWidth, 1));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 8);
	ImGui::InputText("###VCTIn", &vertDistIn);
	if (!enableVertDistLimit) ImGui::EndDisabled();

	ImGui::Checkbox("Facets more coplanar than:", &enableFacetCoplanarityLimit);
	if (!enableFacetCoplanarityLimit) ImGui::BeginDisabled();
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvail().x - txtW * 9;
	ImGui::Dummy(ImVec2(dummyWidth, 1));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 8);
	ImGui::InputText("###FMCTIn", &facetCoplanarityIn);
	if (!enableFacetCoplanarityLimit) ImGui::EndDisabled();

	dummyWidth = txtW * 2;
	ImGui::Dummy(ImVec2(dummyWidth, 1));
	ImGui::SameLine();
	ImGui::Checkbox("Max vertices on a facet:", &enableFacetMaxVertexNum);
	if (!enableFacetMaxVertexNum) ImGui::BeginDisabled();
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvail().x - txtW * 9;
	ImGui::Dummy(ImVec2(dummyWidth, 1));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 8);
	ImGui::InputText("###MVOFIn", &facetMaxVertNumIn);
	if (!enableFacetMaxVertexNum) ImGui::EndDisabled();

	ImGui::Checkbox("Sides more colinear than (deg):", &enableFacetSideColinearityLimit);
	if (!enableFacetSideColinearityLimit) ImGui::BeginDisabled();
	ImGui::SameLine();
	dummyWidth = ImGui::GetContentRegionAvail().x - txtW * 9;
	ImGui::Dummy(ImVec2(dummyWidth, 1));
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 8);
	ImGui::InputText("###SMCTIn", &facetSideColinearityIn);
	if (!enableFacetSideColinearityLimit) ImGui::EndDisabled();

	ImGui::TextDisabled("Collapse results:");
	ImGui::Text(message);
	ImGui::Text(lastAction);

	if (ImGui::Button("Collapse")) {
		CollapseButtonPress(false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Collapse selected")) {
		CollapseButtonPress(true);
	}

	ImGui::End();
}

void ImCollapse::CollapseButtonPress(bool selectionOnly)
{
	// TODO: Cancelling mid execution (requires ImProgress with postponed lambda execution, not yet implemented)
	size_t nbVertexS = interfGeom->GetNbVertex();
	size_t nbFacetS = interfGeom->GetNbFacet();
	size_t nbFacetSS = interfGeom->GetNbSelectedFacets();
	if (enableVertDistLimit && (!Util::getNumber(&vertDist, vertDistIn) || vertDist<=0.0)) {
		ImIOWrappers::InfoPopup("Error", "Invalid vertex distance value. Must be a positive number.");
		return;
	}
	if (enableFacetCoplanarityLimit && (!Util::getNumber(&facetCoplanarity, facetCoplanarityIn) || facetCoplanarity <= 0.0)) {
		ImIOWrappers::InfoPopup("Error", "Invalid planarity threshold value. Must be a positive number.");
		return;
	}
	if (enableFacetMaxVertexNum && (!Util::getNumber(&facetMaxVertNum, facetMaxVertNumIn) || facetMaxVertNum <= 0.0)) {
		ImIOWrappers::InfoPopup("Error", "Invalid max. vertex per facet number. Must be a positive number.");
		return;
	}
	if (enableFacetSideColinearityLimit && (!Util::getNumber(&facetSideColinearity, facetSideColinearityIn) || facetSideColinearity <= 0.0)) {
		ImIOWrappers::InfoPopup("Error", "Invalid linearity threshold value. Must be a positive number.");
		return;
	}
	if (!enableVertDistLimit) vertDist = 0;
	if (!enableFacetCoplanarityLimit) facetCoplanarity = 0;
	if (!enableFacetMaxVertexNum) facetMaxVertNum = 1000000;
	if (!enableFacetSideColinearityLimit) facetSideColinearity = 0;
	{
		LockWrapper lW(mApp->imguiRenderLock);
		if (!mApp->AskToReset(&mApp->worker)) return;
		GLProgress_GUI prg("Collapse", "Please wait");
		prg.SetClosable(false);
		prg.SetVisible(true);
		interfGeom->Collapse(vertDist, facetCoplanarity, facetSideColinearity, facetMaxVertNum, selectionOnly, &mApp->worker, prg);
		interfGeom->CheckCollinear();
		interfGeom->CheckIsolatedVertex();
		mApp->UpdateModelParams();

		// legacy window updates
		if (mApp->vertexCoordinates) mApp->vertexCoordinates->Update();
		mApp->imWnd->Refresh();
		if (mApp->facetCoordinates) mApp->facetCoordinates->UpdateFromSelection();
		if (mApp->profilePlotter) mApp->profilePlotter->Refresh();
		if (mApp->histogramPlotter) mApp->histogramPlotter->Refresh();
	#if defined(MOLFLOW)
		if (((MolFlow*)mApp)->pressureEvolution) ((MolFlow*)mApp)->pressureEvolution->Refresh();
		if (((MolFlow*)mApp)->timewisePlotter) ((MolFlow*)mApp)->timewisePlotter->Refresh();
	#endif
	#if defined(SYNRAD)
		if (mApp->spectrumPlotter) mApp->spectrumPlotter->Refresh();
	#endif

		mApp->worker.MarkToReload();
	}
	message = fmt::format("Selected: {}\nVertex: {}/{}\nFacet: {}/{}", interfGeom->GetNbSelectedFacets(), interfGeom->GetNbVertex(), nbVertexS, interfGeom->GetNbFacet(), nbFacetS);
	lastAction = selectionOnly ? "Collapse Selected" : "Collapse All";
}
