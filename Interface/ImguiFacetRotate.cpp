#include "ImguiFacetRotate.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetRotate::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 51, txtH * 21.75));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Rotate selected facets", &drawn, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
	ImGui::BeginChild("###RSFADM", ImVec2(0,ImGui::GetContentRegionAvail().y-2.75*txtH), true);
	{
		ImGui::TextDisabled("Axis definition mode");
		if (ImGui::RadioButton("X axis", mode == axisX)) mode = axisX;
		if (ImGui::RadioButton("Y axis", mode == axisY)) mode = axisY;
		if (ImGui::RadioButton("Z axis", mode == axisZ)) mode = axisZ;
		ImGui::BeginChild("###RSFADMFF", ImVec2(0,5.75*txtH),true);
		{
			ImGui::TextDisabled("From facet");
			if (ImGui::BeginTable("###RSFFFT", 2, ImGuiTableFlags_SizingFixedFit)) {

				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::RadioButton("U vector", mode == facetU)) mode = facetU;
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::RadioButton("V vector", mode == facetV)) mode = facetV;
				ImGui::TableSetColumnIndex(1);
				if (mode != facetU && mode != facetV && mode != facetN) ImGui::BeginDisabled();
				ImGui::AlignTextToFramePadding();
				ImGui::Text("Of facet #"); ImGui::SameLine();
				ImGui::SetNextItemWidth(txtW * 6);
				ImGui::InputText("###FPDMfacet", &facetIdInput); ImGui::SameLine();
				if (ImGui::Button("<-Get selected")) {
					if (interfGeom->GetNbSelectedFacets() != 1) {
						ImIOWrappers::InfoPopup("Error", "Select exactly one vacet.");
					}
					else {
						for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
							if (interfGeom->GetFacet(i)->selected) {
								facetId = i;
								break;
							}
						}
						facetIdInput = fmt::format("{}", facetId + 1);
					}
				}
				if (mode != facetU && mode != facetV && mode != facetN) ImGui::EndDisabled();
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::RadioButton("Normal vector", mode == facetN)) mode = facetN;
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		if (ImGui::RadioButton("Define by 2 selected vertices", mode == vertices)) mode = vertices;
		if (ImGui::RadioButton("Define by equation:", mode == equation)) mode = equation;
		if (mode != equation) ImGui::BeginDisabled();
		if (ImGui::BeginTable("###RSFADMEQ",8, ImGuiTableFlags_SizingFixedFit)) {
			ImGui::TableNextRow();
			// point row
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding();
			ImGui::Text("Point:");

			ImGui::TableSetColumnIndex(1); ImGui::AlignTextToFramePadding();
			ImGui::Text("a:");
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMa", &aIn);

			ImGui::TableSetColumnIndex(3); ImGui::AlignTextToFramePadding();
			ImGui::Text("b:");
			ImGui::TableSetColumnIndex(4);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMb", &bIn);

			ImGui::TableSetColumnIndex(5); ImGui::AlignTextToFramePadding();
			ImGui::Text("c:");
			ImGui::TableSetColumnIndex(6);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMc", &cIn);

			ImGui::TableSetColumnIndex(7);
			if (ImGui::Button("<-Get base")) {
				GetBaseButtonPress();
			}
			ImGui::TableNextRow();
			// direction row
			ImGui::TableSetColumnIndex(0); ImGui::AlignTextToFramePadding();
			ImGui::Text("Direction:");

			ImGui::TableSetColumnIndex(1); ImGui::AlignTextToFramePadding();
			ImGui::Text("u:"); ImGui::SameLine();
			ImGui::TableSetColumnIndex(2);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMu", &uIn);

			ImGui::TableSetColumnIndex(3); ImGui::AlignTextToFramePadding();
			ImGui::Text("v:"); ImGui::SameLine();
			ImGui::TableSetColumnIndex(4);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMv", &vIn);

			ImGui::TableSetColumnIndex(5); ImGui::AlignTextToFramePadding();
			ImGui::Text("w:"); ImGui::SameLine();
			ImGui::TableSetColumnIndex(6);
			ImGui::SetNextItemWidth(txtW * 6);
			ImGui::InputText("###FPDMw", &wIn);

			ImGui::TableSetColumnIndex(7);
			if (ImGui::Button("<-Calc diff")) {
				CalcDiffButtonPress();
			}
			ImGui::EndTable();
		}
		if (mode != equation) ImGui::EndDisabled();
	}
	ImGui::EndChild();
	if (ImGui::InputTextLLabel("Degrees:", &degIn, 0, txtW * 9)) {
		if (Util::getNumber(&deg, degIn)) {
			rad = deg/180.0*PI;
			radIn = fmt::format("{:.4g}", rad);
		}
	}
	ImGui::SameLine();
	if (ImGui::InputTextLLabel("Radians:", &radIn, 0, txtW * 9)) {
		if (Util::getNumber(&rad, radIn)) {
			deg = rad / PI * 180.0;
			degIn = fmt::format("{:.4g}", deg);
		}
	}
	if (mode == none) ImGui::BeginDisabled();
	if (ImGui::Button("Rotate facet")) {
		RotateFacetButtonPress(false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Copy facet")) {
		RotateFacetButtonPress(true);
	}
	if (mode == none) ImGui::EndDisabled();
	ImGui::End();
}

void ImFacetRotate::RotateFacetButtonPress(bool copy)
{
	if (interfGeom->GetNbSelectedFacets() == 0) {
		ImIOWrappers::InfoPopup("Nothing to rotate", "No facets selected");
		return;
	}
	if (!Util::getNumber(&rad, radIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid angle (radians field)");
		return;
	}
	Vector3d AXIS_P0, AXIS_DIR;
	int selVert1id, selVert2id;
	switch (mode) {
	case axisX:
		AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
		AXIS_DIR = Vector3d(1.0, 0.0, 0.0);
		break;
	case axisY:
		AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
		AXIS_DIR = Vector3d(0.0, 1.0, 0.0);
		break;
	case axisZ:
		AXIS_P0 = Vector3d(0.0, 0.0, 0.0);
		AXIS_DIR = Vector3d(0.0, 0.0, 1.0);
		break;
	case facetU:
		if (!Util::getNumber(&facetId, facetIdInput) || facetId<1 || facetId>interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "Invalid facet number");
			return;
		}
		AXIS_P0 = interfGeom->GetFacet(facetId - 1)->sh.O;
		AXIS_DIR = interfGeom->GetFacet(facetId - 1)->sh.U;
		break;
	case facetV:
		if (!Util::getNumber(&facetId, facetIdInput) || facetId<1 || facetId>interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "Invalid facet number");
			return;
		}
		AXIS_P0 = interfGeom->GetFacet(facetId - 1)->sh.O;
		AXIS_DIR = interfGeom->GetFacet(facetId - 1)->sh.V;
		break;
	case facetN:
		if (!Util::getNumber(&facetId, facetIdInput) || facetId<1 || facetId>interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "Invalid facet number");
			return;
		}
		AXIS_P0 = interfGeom->GetFacet(facetId - 1)->sh.center;
		AXIS_DIR = interfGeom->GetFacet(facetId - 1)->sh.N;
		break;
	case vertices:
		if (interfGeom->GetNbSelectedVertex() != 2) {
			ImIOWrappers::InfoPopup("Can't define axis", "Select exactly 2 vertices");
			return;
		}
		selVert1id = selVert2id = -1;

		for (int i = 0; selVert2id == -1 && i < interfGeom->GetNbVertex(); i++) {
			if (interfGeom->GetVertex(i)->selected) {
				if (selVert1id == -1) {
					selVert1id = i;
				}
				else {
					selVert2id = i;
				}
			}
		}

		AXIS_DIR = *(interfGeom->GetVertex(selVert2id)) - *(interfGeom->GetVertex(selVert1id));
		AXIS_P0 = *(interfGeom->GetVertex(selVert1id));
		break;
	case equation:

		if (!Util::getNumber(&a, aIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid a coordinate");
			return;
		}
		if (!Util::getNumber(&b, bIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid b coordinate");
			return;
		}
		if (!Util::getNumber(&c, cIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid c coordinate");
			return;
		}

		if (!Util::getNumber(&u, uIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid u coordinate");
			return;
		}
		if (!Util::getNumber(&v, vIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid v coordinate");
			return;
		}
		if (!Util::getNumber(&w, wIn)) {
			ImIOWrappers::InfoPopup("Error", "Invalid w coordinate");
			return;
		}

		if ((u == 0.0) && (v == 0.0) && (w == 0.0)) {
			ImIOWrappers::InfoPopup("Error", "u, v, w are all zero. That's not a vector.");
			return;
		}

		AXIS_P0 = Vector3d(a, b, c);
		AXIS_DIR = Vector3d(u, v, w);
		break;
	default:
		ImIOWrappers::InfoPopup("Error", "Select an axis definition mode.");
		return;
	}

	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		interfGeom->RotateSelectedFacets(AXIS_P0, AXIS_DIR, rad, copy, &mApp->worker);
		mApp->worker.MarkToReload();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
		mApp->changedSinceSave = true;
	}
}

void ImFacetRotate::GetBaseButtonPress()
{
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	mode = equation;
	int selVertexId = -1;
	for (int i = 0; selVertexId == -1 && i < interfGeom->GetNbVertex(); i++) {
		if (interfGeom->GetVertex(i)->selected) {
			selVertexId = i;
		}
	}
	Vector3d* selVertex = interfGeom->GetVertex(selVertexId);
	a = selVertex->x;
	b = selVertex->y;
	c = selVertex->z;

	aIn = fmt::format("{}", a);
	bIn = fmt::format("{}", b);
	cIn = fmt::format("{}", c);
}

void ImFacetRotate::CalcDiffButtonPress()
{
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	if (!Util::getNumber(&a, aIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid a coordinate");
		return;
	}
	if (!Util::getNumber(&b, bIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid b coordinate");
		return;
	}
	if (!Util::getNumber(&c, cIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid c coordinate");
		return;
	}
	mode = equation;
	int selVertexId = -1;
	for (int i = 0; selVertexId == -1 && i < interfGeom->GetNbVertex(); i++) {
		if (interfGeom->GetVertex(i)->selected) {
			selVertexId = i;
		}
	}
	Vector3d* selVertex = interfGeom->GetVertex(selVertexId);
	u = selVertex->x-a;
	v = selVertex->y-b;
	w = selVertex->z-c;

	uIn = fmt::format("{}", u);
	vIn = fmt::format("{}", v);
	wIn = fmt::format("{}", w);
}
