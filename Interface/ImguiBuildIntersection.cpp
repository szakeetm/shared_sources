#include "ImguiBuildIntersection.h"
#include "ImguiWindow.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiPopup.h"
#include "Interface.h"

void ImBuildIntersect::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 40, txtH * 8));
	ImGui::Begin("Build Intersection", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);
	ImGui::TextWrapped(
		"Select all facets that form part of the intersection. Then select vertices that you'd like to ensure are being kept. Facets without any selected vertex won't be altered"
	);
	if (ImGui::Button("Build Intersection")) BuildButtonPress();
	ImGui::SameLine();
	bool isDisabled = !undoEnabled;
	if (isDisabled) ImGui::BeginDisabled();
	if (ImGui::Button("Undo")) UndoButtonPress();
	if (isDisabled) ImGui::EndDisabled();
	ImGui::Text(message);
	ImGui::End();
}

void ImBuildIntersect::BuildButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() < 2) {
		ImIOWrappers::InfoPopup("Can't create intersection", "Select at least 2 facets");
		return;
	}
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		ClearUndoFacets();
		nbCreated = 0;
		deletedFacetList = interfGeom->BuildIntersection(&nbCreated);
		nbFacet = interfGeom->GetNbFacet();
		std::stringstream tmp;
		tmp << deletedFacetList.size() << " facets intersected, creating " << nbCreated << " new.";
		message = (tmp.str().c_str());
		if (!deletedFacetList.empty()) undoEnabled = true;
		mApp->worker.MarkToReload();
		mApp->UpdateModelParams();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void ImBuildIntersect::UndoButtonPress()
{
	auto f = [this](bool toEnd) {
		interfGeom->RestoreFacets(deletedFacetList, toEnd);
		deletedFacetList.clear();
		undoEnabled = false;
		message = "";
		LockWrapper lW(mApp->imguiRenderLock);
		mApp->worker.MarkToReload();
		mApp->UpdateModelParams();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
		};

	if (nbFacet == interfGeom->GetNbFacet()) { //Assume no change since the split operation
		std::vector<size_t> newlyCreatedList;
		for (size_t index = (interfGeom->GetNbFacet() - nbCreated); index < interfGeom->GetNbFacet(); index++) {
			newlyCreatedList.push_back(index);
		}
		{
			LockWrapper lW(mApp->imguiRenderLock);
			interfGeom->RemoveFacets(newlyCreatedList);
		}
		f(true);
	}
	else {
		mApp->imWnd->popup.Open("Undo build?", "Geometry changed since intersecting, restore to end without deleting the newly created facets?",
			{std::make_shared<ImIOWrappers::ImButtonFuncInt>("Ok", f, false, ImGuiKey_Enter, ImGuiKey_KeypadEnter),
			std::make_shared<ImIOWrappers::ImButtonInt>("Cancel", ImGuiKey_Escape)}
			);
	}
}

void ImBuildIntersect::ClearUndoFacets()
{
	for (DeletedFacet delFacet : deletedFacetList)
		SAFE_DELETE(delFacet.f);
	deletedFacetList.clear();
	message = "";
}
