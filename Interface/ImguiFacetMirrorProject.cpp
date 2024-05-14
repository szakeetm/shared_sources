#include "ImguiFacetMirrorProject.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetMirrorProject::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 16));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Mirror/project selected facets###MPFW", &drawn, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);
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
	if (mode != planeOfFacet) ImGui::EndDisabled();
	if (ImGui::Button("<-Get selected")) {
		if (interfGeom->GetNbSelectedFacets() != 1) {
			ImIOWrappers::InfoPopup("Error", "Select exactly one facet.");
		}
		else {
			mode = planeOfFacet;
			for (int i = 0; i < interfGeom->GetNbFacet(); i++) {
				if (interfGeom->GetFacet(i)->selected) {
					facetId = i;
					break;
				}
			}
			facetIdInput = fmt::format("{}", facetId + 1);
		}
	}
	if(ImGui::RadioButton("Define by 3 selected vertices", mode == byVerts)) mode = byVerts;
	if(ImGui::RadioButton("Define by plane equation", mode == byEqation)) mode = byEqation;
	if (mode != byEqation) ImGui::BeginDisabled();

	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMX", &aInput); ImGui::SameLine(); ImGui::Text("*X+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMY", &bInput); ImGui::SameLine(); ImGui::Text("*Y+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMZ", &cInput); ImGui::SameLine(); ImGui::Text("*Z+"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###FPDMW", &dInput); ImGui::SameLine(); ImGui::Text("=0");

	if (mode != byEqation) ImGui::EndDisabled();

	ImGui::EndChild();
	if (ImGui::Button("Mirror facet")) {
		DoMirrorProject(Mirror, false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Copy mirror facet")) {
		DoMirrorProject(Mirror, true);
	}
	if (ImGui::Button("Project facet")) {
		DoMirrorProject(Project, false);
	}ImGui::SameLine();
	if (ImGui::Button("Copy project facet")) {
		DoMirrorProject(Project, true);
	}ImGui::SameLine();
	if (!enableUndo) ImGui::BeginDisabled();
	bool undid = false;
	if (ImGui::Button("Undo projection")) {
		UndoProjection();
		undid = true;
	}
	if (!enableUndo && !undid) ImGui::EndDisabled();
	ImGui::End();
}

void ImFacetMirrorProject::Clear()
{
	undoPoints.clear();
	enableUndo = false;
}

void ImFacetMirrorProject::DoMirrorProject(Action action, bool copy)
{
	if (interfGeom->GetNbSelectedFacets() == 0) {
		ImIOWrappers::InfoPopup("Nothing to mirror", "No facets selected");
		return;
	}
	Vector3d P0, N;
	double nN2;
	Vector3d U2, V2, N2;
	int v1Id = -1;
	int v2Id = -1;
	int v3Id = -1;

	switch (mode) {
	case xy:
		P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
		N.x = 0.0; N.y = 0.0; N.z = 1.0;
		break;
	case xz:
		P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
		N.x = 0.0; N.y = 1.0; N.z = 0.0;
		break;
	case yz:
		P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
		N.x = 1.0; N.y = 0.0; N.z = 0.0;
		break;
	case planeOfFacet:
		if (!Util::getNumber(&facetId, facetIdInput) || facetId<1 || facetId>interfGeom->GetNbFacet()) {
			ImIOWrappers::InfoPopup("Error", "Invalid facet number");
			return;
		}
		P0 = *interfGeom->GetVertex(interfGeom->GetFacet(facetId - 1)->indices[0]);
		N = interfGeom->GetFacet(facetId - 1)->sh.N;
		break;
	case byVerts:
		if (interfGeom->GetNbSelectedVertex() != 3) {
			ImIOWrappers::InfoPopup("Can't define plane", "Select exactly 3 vertices");
			return;
		}

		for (int i = 0; v3Id == -1 && i < interfGeom->GetNbVertex(); i++) {
			if (interfGeom->GetVertex(i)->selected) {
				if (v1Id == -1) v1Id = i;
				else if (v2Id == -1) v2Id = i;
				else v3Id = i;
			}
		}

		U2 = (*interfGeom->GetVertex(v1Id) - *interfGeom->GetVertex(v2Id)).Normalized();
		V2 = (*interfGeom->GetVertex(v1Id) - *interfGeom->GetVertex(v3Id)).Normalized();
		N2 = CrossProduct(V2, U2);
		nN2 = N2.Norme();
		if (nN2 < 1e-8) {
			ImIOWrappers::InfoPopup("Can't define plane", "The 3 selected vertices are on a line.");
			return;
		}
		N2 = 1.0 / nN2 * N2; // Normalize N2
		N = N2;
		P0 = *interfGeom->GetVertex(v1Id);
		break;
	case byEqation:
		if (!Util::getNumber(&a, aInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid A coefficient");
			return;
		}
		if (!Util::getNumber(&b, bInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid B coefficient");
			return;
		}
		if (!Util::getNumber(&c, cInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid C coefficient");
			return;
		}
		if (!Util::getNumber(&d, dInput)) {
			ImIOWrappers::InfoPopup("Error", "Invalid D coefficient");
			return;
		}
		if ((a == 0.0) && (b == 0.0) && (c == 0.0)) {
			ImIOWrappers::InfoPopup("Error", "A, B, C are all zero.That's not a plane.");
			return;
		}
		N.x = a; N.y = b; N.z = c;
		N=N.Normalized();
		P0.x = 0.0; P0.y = 0.0; P0.z = 0.0;
		if (a != 0.0) P0.x = -d / a;
		else if (b != 0.0) P0.y = -d / b;
		else if (c != 0.0) P0.z = -d / c;
		break;
	default:
		ImIOWrappers::InfoPopup("Error", "Select a plane definition mode.");
		return;
	}
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {
		undoPoints = interfGeom->MirrorProjectSelectedFacets(P0, N, action == Project, copy, &mApp->worker);
		enableUndo = action == Project && !copy;
		mApp->worker.MarkToReload();
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void ImFacetMirrorProject::UndoProjection()
{
	LockWrapper lW(mApp->imguiRenderLock);
	if (!mApp->AskToReset(&mApp->worker)) return;
	for (UndoPoint oriPoint : undoPoints) {
		if (oriPoint.oriId < interfGeom->GetNbVertex()) interfGeom->GetVertex(oriPoint.oriId)->SetLocation(oriPoint.oriPos);
	}
	enableUndo = false;
	interfGeom->InitializeGeometry();
	mApp->worker.MarkToReload();
	mApp->UpdateFacetlistSelected();
	mApp->UpdateViewers();
}
