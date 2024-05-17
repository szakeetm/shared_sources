#include "ImguiMeasureForce.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "Interface.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

void ImMeasureForce::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 55, txtH * 9.5));
	ImGui::Begin("Measure forces", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::Checkbox("Enable force measurement (has performance impact)", &enableForceMeasurement);

	ImGui::BeginChild("###MFchild", ImVec2(0,ImGui::GetContentRegionAvail().y-txtH*1.5), true);
	
	ImGui::TextDisabled("Torque relative to...");

	if (ImGui::BeginTable("###MFT", 6, ImGuiTableFlags_SizingStretchProp)) {

		ImGui::TableNextRow();
		
		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("mx0");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###mx0", &mx0I);

		ImGui::TableSetColumnIndex(2);
		ImGui::Text("my0");
		ImGui::TableSetColumnIndex(3);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###my0", &my0I);

		ImGui::TableSetColumnIndex(4);
		ImGui::Text("mz0");
		ImGui::TableSetColumnIndex(5);
		ImGui::SetNextItemWidth(txtW * 12);
		ImGui::InputText("###mz0", &mz0I);

		ImGui::EndTable();
	}
	if (ImGui::Button("Selected vertex")) {
		SelectedVertexButtonPress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Center of selected facet")) {
		FacetCenterButtonPress();
	}

	ImGui::EndChild();
	ImGui::PlaceAtRegionCenter(" Apply ");
	if (ImGui::Button("Apply")) {
		ApplyButtonPress();
	}
	ImGui::End();
}

void ImMeasureForce::Update()
{
	enableForceMeasurement = mApp->worker.model->sp.enableForceMeasurement;
	mx = mApp->worker.model->sp.torqueRefPoint.x;
	my = mApp->worker.model->sp.torqueRefPoint.y;
	mz = mApp->worker.model->sp.torqueRefPoint.z;
	mx0I = fmt::format("{:.3g}", mx);
	my0I = fmt::format("{:.3g}", my);
	mz0I = fmt::format("{:.3g}", mz);
}

void ImMeasureForce::OnShow()
{
	Update();
}

void ImMeasureForce::SelectedVertexButtonPress()
{
	size_t nbs = interfGeom->GetNbSelectedVertex();
	if (nbs != 1) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Select exactly one vertex\n(You have selected {}).", nbs));
		return;
	}
	for (int i = 0; i < interfGeom->GetNbVertex(); i++) {
		if (interfGeom->GetVertex(i)->selected) {
			mx = interfGeom->GetVertex(i)->x;
			mx0I = fmt::format("{:.3g}", mx);
			my = interfGeom->GetVertex(i)->y;
			my0I = fmt::format("{:.3g}", my);
			mz = interfGeom->GetVertex(i)->z;
			mz0I = fmt::format("{:.3g}", mz);
			break;
		}
	}
}

void ImMeasureForce::FacetCenterButtonPress()
{
	size_t nbs = interfGeom->GetNbSelectedFacets();
	if (nbs != 1) {
		ImIOWrappers::InfoPopup("Error", fmt::format("Select exactly one facet\n(You have selected {}).", nbs));
		return;
	}
	for (size_t i = 0; i < interfGeom->GetNbFacet(); i++) {
		auto f = interfGeom->GetFacet(i);
		if (f->selected) {
			mx = f->sh.center.x;
			mx0I = fmt::format("{:.3g}", mx);
			my = f->sh.center.y;
			my0I = fmt::format("{:.3g}", my);
			mz = f->sh.center.z;
			mz0I = fmt::format("{:.3g}", mz);
			break;
		}
	}
}

void ImMeasureForce::ApplyButtonPress()
{
	if (enableForceMeasurement) {
		if (!Util::getNumber(&mx, mx0I)) {
			ImIOWrappers::InfoPopup("Error", "Invalix mx0 coordinate");
			return;
		}
		if (!Util::getNumber(&my, my0I)) {
			ImIOWrappers::InfoPopup("Error", "Invalix my0 coordinate");
			return;
		}
		if (!Util::getNumber(&mz, mz0I)) {
			ImIOWrappers::InfoPopup("Error", "Invalix mz0 coordinate");
			return;
		}
	}
	Apply();
}

void ImMeasureForce::Apply()
{
	LockWrapper imLock(mApp->imguiRenderLock);
	if (!mApp->AskToReset()) return;
	mApp->worker.model->sp.enableForceMeasurement = enableForceMeasurement;
	if (enableForceMeasurement) {
		mApp->worker.model->sp.torqueRefPoint = Vector3d(mx, my, mz);
	}
	mApp->worker.MarkToReload();
	mApp->changedSinceSave = true;
}
