#include "ImguiFacetScale.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "imgui_stdlib/imgui_stdlib.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetScale::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(5 * txtW, 3 * txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(txtW * 55, txtH * 14), ImGuiCond_FirstUseEver);
	ImGui::Begin("Scale selected facets", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize );

	ImGui::BeginChild("##FSC1", ImVec2(0, 6 * txtH), true);
	ImGui::TextDisabled("Invariant point definition mode");
	if (ImGui::RadioButton("###SSF", invPt == Coords)) invPt = Coords;
	if (invPt != Coords) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel("X=", &invariantPointInput[X], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Y=", &invariantPointInput[Y], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Z=", &invariantPointInput[Z], 0, txtW * 6);
	if (invPt != Coords) ImGui::EndDisabled();

	if (ImGui::RadioButton("Selected vertex", invPt == Vertex)) invPt = Vertex;
	if (ImGui::RadioButton("Center of selected facet #", invPt == Facet)) invPt = Facet;
	if (invPt != Facet) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	ImGui::InputText("###facetN", &facetIdInput);
	if (invPt != Facet) ImGui::EndDisabled();
	ImGui::SameLine();
	if (ImGui::Button("<-Get selected")) {
		if (interfGeom->GetNbSelectedFacets() != 1) {
			ImIOWrappers::InfoPopup("Error", "Select exactly one facet.");
		}
		else {
			for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
				if (interfGeom->GetFacet(i)->selected) {
					facetId = i;
					break;
				}
			}
			facetIdInput = fmt::format("{}", facetId+1);
			invPt = Facet;
		}
	}

	ImGui::EndChild();
	ImGui::BeginChild("##FSC2", ImVec2(0, 4.5 * txtH), true);
	ImGui::TextDisabled("Scale factor");
	if (ImGui::RadioButton("Uniform", scaleFac==Uniform)) scaleFac=Uniform;
	if (scaleFac != Uniform) ImGui::BeginDisabled();
	ImGui::SameLine();
	if (ImGui::InputTextLLabel(" ", &uniformScaleNumeratorInput) && Util::getNumber(&uniformScaleNumerator, uniformScaleNumeratorInput)) {
		uniformScaleDenumerator = 1 / uniformScaleNumerator;
		uniformScaleDenominatorInput = fmt::format("{}", uniformScaleDenumerator);
	}
	ImGui::SameLine();
	ImGui::AlignTextToFramePadding();
	ImGui::Text("(=1/");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(100);
	if (ImGui::InputText("###1by", &uniformScaleDenominatorInput) && Util::getNumber(&uniformScaleDenumerator, uniformScaleDenominatorInput)) {
		uniformScaleNumerator = 1 / uniformScaleDenumerator;
		uniformScaleNumeratorInput = fmt::format("{}", uniformScaleNumerator);
	}
	ImGui::SameLine(); ImGui::Text(")");
	if (scaleFac != Uniform) ImGui::EndDisabled();
	if (ImGui::RadioButton("Distorted", scaleFac==Distorted)) scaleFac=Distorted;
	if (scaleFac != Distorted) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel("X:", &distortedScaleFactorInput[X], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Y:", &distortedScaleFactorInput[Y], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Z:", &distortedScaleFactorInput[Z], 0, txtW * 6);
	if (scaleFac != Distorted) ImGui::EndDisabled();
	ImGui::EndChild();

	if (ImGui::Button("Scale facet")) {
		Apply(Scale);
	}
	ImGui::SameLine();
	if (ImGui::Button("Copy facet")) {
		Apply(Copy);
	}
	ImGui::End();
}

void ImFacetScale::GetSelectedFacetButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet.");
		return;
	}
	for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
		if (interfGeom->GetFacet(i)->selected) {
			facetId = i;
			facetIdInput = std::to_string(facetId);
			return;
		}
	}
}

void ImFacetScale::Apply(Action action)
{
	if (interfGeom->GetNbSelectedFacets() == 0) {
		ImIOWrappers::InfoPopup("Nothing to scale", "No facets selected");
		return;
	}
	Vector3d invariant;
	bool found;

	switch (invPt) {
	case Coords:
		if (!Util::getNumber(&invariant.x, invariantPointInput[X])) {
			ImIOWrappers::InfoPopup("Error", "Invalid X coordinate");
			return;
		}
		if (!Util::getNumber(&invariant.y, invariantPointInput[Y])) {
			ImIOWrappers::InfoPopup("Error", "Invalid Y coordinate");
			return;
		}
		if (!Util::getNumber(&invariant.z, invariantPointInput[Z])) {
			ImIOWrappers::InfoPopup("Error", "Invalid Z coordinate");
			return;
		}
		break;
	case Vertex:
		if (!(interfGeom->GetNbSelectedVertex() == 1)) {
			ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
			return;
		}
		found = false;
		for (int i = 0; !found && i < interfGeom->GetNbVertex(); i++) {
			if (interfGeom->GetVertex(i)->selected)
				invariant = *(interfGeom->GetVertex(i));
		}
		break;
	case Facet:
		if (!Util::getNumber(&facetId, facetIdInput) || facetId<1 || facetId>interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "Invalid facet number");
			return;
		}
		invariant = interfGeom->GetFacet(facetId - 1)->sh.center;
		break;
	default:
		ImIOWrappers::InfoPopup("Error", "Select an invariant definition mode.");
		return;
	}
	
	switch (scaleFac) {
	case Uniform:
		if (!Util::getNumber(&uniformScaleNumerator, uniformScaleNumeratorInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid scale factor number");
			return;
		}
		break;
	case Distorted:
		if (!Util::getNumber(&distortedScaleFactor[X], distortedScaleFactorInput[X])) {
			ImIOWrappers::InfoPopup("Error", "Invalid X scale factor number");
			return;
		}
		if (!Util::getNumber(&distortedScaleFactor[Y], distortedScaleFactorInput[Y])) {
			ImIOWrappers::InfoPopup("Error", "Invalid Y scale factor number");
			return;
		}
		if (!Util::getNumber(&distortedScaleFactor[Z], distortedScaleFactorInput[Z])) {
			ImIOWrappers::InfoPopup("Error", "Invalid Z scale factor number");
			return;
		}
		break;
	}
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		if (scaleFac==Uniform) distortedScaleFactor[X] = distortedScaleFactor[Y] = distortedScaleFactor[Z] = uniformScaleNumerator;
		interfGeom->ScaleSelectedFacets(invariant, distortedScaleFactor[X], distortedScaleFactor[Y], distortedScaleFactor[Z], action==Copy, &mApp->worker);
		mApp->UpdateModelParams();
		mApp->worker.MarkToReload();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
		mApp->changedSinceSave = true;
	}
}
