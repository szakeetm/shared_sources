#include "ImguiFacetExtrude.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "ImguiWindow.h"

#if defined(MOLFLOW)
#include "../../src/MolFlow.h"
#endif

#if defined(SYNRAD)
#include "../src/SynRad.h"
#endif

void ImFacetExtrude::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 32));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Extrude Facet", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::BeginChild("###EF1", ImVec2(0,txtH*4.25), true);
	ImGui::TextDisabled("Towards/against normal");
	if (ImGui::RadioButton("Towards normal", mode == facetNormal)) mode = facetNormal;
	ImGui::SameLine();
	if (ImGui::RadioButton("Against normal", mode == facetAntinormal)) mode = facetAntinormal;
	if (!(mode == facetNormal || mode == facetAntinormal)) ImGui::BeginDisabled();
	ImGui::InputTextLLabel("extrusion length:", &facetLengthInput, 0, txtW * 6);
	if (!(mode == facetNormal || mode == facetAntinormal)) ImGui::EndDisabled();
	ImGui::EndChild();

	ImGui::BeginChild("###EF2", ImVec2(0, txtH * 7), true);
	ImGui::TextDisabled("Along straight path");
	if (ImGui::RadioButton("Direction vector", mode == directionVector)) mode = directionVector;
	if (!(mode == directionVector)) ImGui::BeginDisabled();
	ImGui::Text("dX:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DX", &pathDXInput); ImGui::SameLine();
	ImGui::Text("cm\tdY:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DY", &pathDYInput); ImGui::SameLine();
	ImGui::Text("cm\tdZ:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DZ", &pathDZInput); ImGui::SameLine();
	ImGui::Text("cm");
	if (ImGui::Button("Get Base Vertex")) {
		GetBaseButtonPress();
	}
	ImGui::SameLine();
	if (ImGui::Button("Get Dir. Vertex")) {
		GetDirectionButtonPress();
	}
	ImGui::Text(fmt::format("Base vert.: {}\tDir. vert.: {}", baseId == -1 ? "none" : std::to_string(baseId), dirId == -1 ? "none" : std::to_string(dirId)));
	if (!(mode == directionVector)) ImGui::EndDisabled();
	ImGui::EndChild();

	ImGui::BeginChild("###EF3", ImVec2(0, txtH * 17), true);
	ImGui::TextDisabled("Along curve");
	if (ImGui::RadioButton("Towards normal", mode == curveNormal)) mode = curveNormal;
	ImGui::SameLine();
	if (ImGui::RadioButton("Against normal", mode == curveAntinormal)) mode = curveAntinormal;

	if (!(mode == curveNormal || mode == curveAntinormal)) ImGui::BeginDisabled();
	ImGui::Text("Radius base:");
	ImGui::SameLine();
	ImGui::Text(facetInfo);
	if (ImGui::Button("Facet center")) {
		FacetCenterButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("Facet index1")) {
		FacetIndex1ButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("From selected")) {
		CurveGetBaseButtonPress();
	}
	ImGui::Text("X0:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3X0", &curveX0Input); ImGui::SameLine();
	ImGui::Text("cm\tY0:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3Y0", &curveY0Input); ImGui::SameLine();
	ImGui::Text("cm\tZ0:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3Z0", &curveZ0Input); ImGui::SameLine();
	ImGui::Text("cm");

	ImGui::Text("Radius direction");
	if (ImGui::Button("Facet U")) {
		CurveFacetUButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("Facet V")) {
		CurveFacetVButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("Towards selected")) {
		CurveGetDirectionButtonPress();
	}
	
	if (ImGui::Button("Facet N x X")) {
		FacetNXButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("Facet N x Y")) {
		FacetNYButtonPress();
	} ImGui::SameLine();
	if (ImGui::Button("Facet N x Z")) {
		FacetNZButtonPress();
	}

	ImGui::Text("dX:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DX", &curveDXInput); ImGui::SameLine();
	ImGui::Text("cm\tdY:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DY", &curveDYInput); ImGui::SameLine();
	ImGui::Text("cm\tdZ:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DZ", &curveDZInput); ImGui::SameLine();
	ImGui::Text("cm");

	if (ImGui::BeginTable("###EF3T", 5, ImGuiTableFlags_SizingFixedFit)) {

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Radius:");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 6);
		if (ImGui::InputText("###EF3TR", &radiusInput)) {
			if (Util::getNumber(&radius, radiusInput) && Util::getNumber(&angleRad, angleRadInput)) {
				curveLength = angleRad * radius;
				curveLengthInput = fmt::format("{}", curveLength);
				angleDeg = angleRad * 180 / PI;
				angleDegInput = fmt::format("{}", angleDeg);
			}
		}
		ImGui::TableSetColumnIndex(2);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("cm");
		
		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Total angle:");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 6);
		if (ImGui::InputText("###EF3TAD", &angleDegInput, 0)) {
			if (Util::getNumber(&angleDeg, angleDegInput)) {
				angleRad = angleDeg / 180 * PI;
				angleRadInput = fmt::format("{}", angleRad);
				if (Util::getNumber(&radius, radiusInput)) {
					curveLength = angleDeg / 180 * PI * radius;
					curveLengthInput = fmt::format("{}", curveLength);
				}
			}
		}
		ImGui::TableSetColumnIndex(2);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("deg");
		ImGui::TableSetColumnIndex(3);
		ImGui::SetNextItemWidth(txtW * 6);
		if (ImGui::InputText("###EF3RAD", &angleRadInput)) {
			if (Util::getNumber(&angleRad, angleRadInput)) {
				angleDeg = angleRad / PI * 180;
				angleDegInput = fmt::format("{}", angleDeg);
				if (Util::getNumber(&radius, radiusInput)) {
					curveLength = angleRad * radius;
					curveLengthInput = fmt::format("{}", curveLength);
				}
			}

		}
		ImGui::TableSetColumnIndex(4);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("rad");

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Total length:");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 6);
		if (ImGui::InputText("###FE3TL", &curveLengthInput)) {
			if (Util::getNumber(&curveLength, curveLengthInput) && Util::getNumber(&angleRad, angleRadInput)) {
				angleDeg = curveLength / radius * 180 / PI;
				angleDegInput = fmt::format("{}", angleDeg);
				angleRad = curveLength/radius;
				angleRadInput = fmt::format("{}", angleRad);
			}
		}
		ImGui::TableSetColumnIndex(2);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("cm");

		ImGui::TableNextRow();

		ImGui::TableSetColumnIndex(0);
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Steps:");
		ImGui::TableSetColumnIndex(1);
		ImGui::SetNextItemWidth(txtW * 6);
		ImGui::InputText("###FE3TS", &stepsInput, 0);

		ImGui::EndTable();
	}


	if (!(mode == curveNormal || mode == curveAntinormal)) ImGui::EndDisabled();
	ImGui::EndChild();

	if (ImGui::Button("Extrude")) {
		ExtrudeButtonPress();
	}

	ImGui::End();
}

std::optional<size_t> ImFacetExtrude::AssertOneVertexSelected()
{
	auto selectedVertices = interfGeom->GetSelectedVertices();
	if (selectedVertices.size() == 0) {
		ImIOWrappers::InfoPopup("Can't define direction", "No vertex selected");
		return std::nullopt;
	}
	else if (selectedVertices.size() > 1) {
		ImIOWrappers::InfoPopup("Can't define direction", "More than one vertex is selected");
		return std::nullopt;
	}
	else return selectedVertices[0];
}

std::optional<size_t> ImFacetExtrude::AssertOneFacetSelected()
{
	auto selectedFacets = interfGeom->GetSelectedFacets();
	if (selectedFacets.size() == 0) {
		ImIOWrappers::InfoPopup("Can't define source", "No facet selected");
		return std::nullopt;
	}
	else if (selectedFacets.size() > 1) {
		ImIOWrappers::InfoPopup("Can't define source", "More than one facet is selected");
		return std::nullopt;
	}
	else return selectedFacets[0];
}

void ImFacetExtrude::PreProcessExtrude() {
	
	if (mode == directionVector && !Util::getNumber(&pathDX, pathDXInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid direction vector dX value");
		return;
	}
	if (mode == directionVector && !Util::getNumber(&pathDY, pathDYInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid direction vector dY value");
		return;
	}
	if (mode == directionVector && !Util::getNumber(&pathDZ, pathDZInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid direction vector dZ value");
		return;
	}

	if ((mode == facetNormal || mode == facetAntinormal)
		&& !Util::getNumber(&facetLength, facetLengthInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid extrusion length value");
		return;
	}

	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveX0, curveX0Input)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius base X0 value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveY0, curveY0Input)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius base Y0 value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveZ0, curveZ0Input)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius base Z0 value");
		return;
	}

	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveDX, curveDXInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius direction dX value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveDY, curveDYInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius direction dY value");
		return;
	}if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveDZ, curveDZInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid radius direction dZ value");
		return;
	}

	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&radius, radiusInput)){
		ImIOWrappers::InfoPopup("Error","Invalid radius length value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&angleDeg, angleDegInput)){
		ImIOWrappers::InfoPopup("Error", "Invalid total angle (deg) value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&curveLength, curveLengthInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid total length value");
		return;
	}
	if ((mode == curveNormal || mode == curveAntinormal)
		&& !Util::getNumber(&steps, stepsInput)) {
		ImIOWrappers::InfoPopup("Error", "Invalid 'number of steps' value");
		return;
	}
	/*
	int mode;
	if (towardsNormalCheckbox->GetState() || againstNormalCheckbox->GetState()) mode = 1; //By distance
	else if (offsetCheckbox->GetState()) mode = 2; // By offset vector
	else mode = 3; //Along curve
	*/
	std::string warning = "";
	if ((mode == directionVector) && pathDX * pathDX + pathDY * pathDY + pathDZ * pathDZ < 1E-8) {
		ImIOWrappers::InfoPopup("Error", "Direction is a null-vector");
		return;
	} if ((mode == curveAntinormal || mode == curveNormal) && curveDX * curveDX + curveDY * curveDY + curveDZ * curveDZ < 1E-8) {
		ImIOWrappers::InfoPopup("Error", "Direction is a null-vector");
		return;
	} if ((mode == curveAntinormal || mode == curveNormal) && std::abs(radius) < 1E-8) {
		ImIOWrappers::InfoPopup("Error", "Radius length can't be 0");
		return;
	} if ((mode == facetNormal || mode == facetAntinormal) && std::abs(facetLength) < 1E-8) {
		ImIOWrappers::InfoPopup("Error", "Extrusion length can't be 0");
		return;
	} if ((mode == curveAntinormal || mode == curveNormal) && (std::abs(angleDeg) < 1E-8 || angleDeg > 360)) {
		ImIOWrappers::InfoPopup("Error", "Total angle can't be 0");
		return;
	} if ((mode == curveAntinormal || mode == curveNormal) && !(steps > 0)) {
		ImIOWrappers::InfoPopup("Error", "Invalid number of steps");
		return;
	} if ((mode == curveAntinormal || mode == curveNormal) && (angleDeg < -360 || angleDeg>360)) {
		warning.append("Total angle outside -360..+360 degree. Are you sure?\n");
	} if ((mode == curveAntinormal || mode == curveNormal) && (steps > 50)) {
		warning.append(fmt::format("Are you sure you want to sweep in {} steps?\n", steps));
	}
	if (warning != "") {
		if (warning[warning.size() - 1] == '\n') warning.erase(warning.end() - 1);
		mApp->imWnd->popup.Open("Warning", warning, { std::make_shared<ImIOWrappers::ImButtonFunc>("Yes", ([this]()->void { DoExtrude(); }), ImGuiKey_Enter, ImGuiKey_KeypadEnter), std::make_shared<ImIOWrappers::ImButtonInt>("Cancel") });
		return;
	}
	DoExtrude();
}

void ImFacetExtrude::DoExtrude()
{
	LockWrapper lW(mApp->imguiRenderLock);
	if (mApp->AskToReset()) {

		Vector3d radiusBase, offsetORradiusdir;
		radiusBase.x = curveX0;
		radiusBase.y = curveY0;
		radiusBase.z = curveZ0;
		if (mode == directionVector) {
			offsetORradiusdir.x = pathDX;
			offsetORradiusdir.y = pathDY;
			offsetORradiusdir.z = pathDZ;
		}
		else if (mode == curveAntinormal || mode == curveNormal) {
			offsetORradiusdir.x = curveDX;
			offsetORradiusdir.y = curveDY;
			offsetORradiusdir.z = curveDZ;
		}
		double distanceORradius;
		if ((mode == facetNormal || mode == facetAntinormal)) distanceORradius = facetLength;
		else if ((mode == curveAntinormal || mode == curveNormal)) distanceORradius = radius;

		int extrudeMode;
		if (mode == facetNormal || mode == facetAntinormal) extrudeMode = 1;
		else if (mode == directionVector) extrudeMode = 2;
		else extrudeMode = 3;

		interfGeom->Extrude(extrudeMode, radiusBase, offsetORradiusdir,
			mode == facetAntinormal || mode == curveAntinormal,
			((mode == facetNormal || mode == facetAntinormal)) ? facetLength : radius,
			angleDeg / 180 * PI, steps);

		mApp->worker.MarkToReload();
		mApp->changedSinceSave = true;
		mApp->UpdateFacetlistSelected();
		mApp->UpdateViewers();
	}
}

void ImFacetExtrude::ExtrudeButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() == 0) {
		ImIOWrappers::InfoPopup("Nothing to move", "No facets selected");
		return;
	}
	else if (interfGeom->GetNbSelectedFacets() > 1) {
		mApp->imWnd->popup.Open("Extrusion of more than one facet", fmt::format("Extrude {} facets at once?", interfGeom->GetNbSelectedFacets()),
			{ std::make_shared<ImIOWrappers::ImButtonFunc>("Yes", ([this]()->void { PreProcessExtrude(); }), ImGuiKey_Enter, ImGuiKey_KeypadEnter), std::make_shared<ImIOWrappers::ImButtonInt>("Cancel") });
		return;
	}
	PreProcessExtrude();
}

void ImFacetExtrude::GetBaseButtonPress()
{
	if (auto foundId = AssertOneVertexSelected()) {
		baseId = *foundId;
		if (dirId > 0 && dirId < interfGeom->GetNbVertex()) {
			pathDX=(interfGeom->GetVertex(dirId)->x - interfGeom->GetVertex(baseId)->x);
			pathDY=(interfGeom->GetVertex(dirId)->y - interfGeom->GetVertex(baseId)->y);
			pathDZ=(interfGeom->GetVertex(dirId)->z - interfGeom->GetVertex(baseId)->z);

			pathDXInput = fmt::format("{}", pathDX);
			pathDYInput = fmt::format("{}", pathDY);
			pathDZInput = fmt::format("{}", pathDZ);
		}
	}
}

void ImFacetExtrude::GetDirectionButtonPress()
{
	if (auto foundId = AssertOneVertexSelected()) {
		dirId = *foundId;
		if (baseId > 0 && baseId < interfGeom->GetNbVertex()) {
			pathDX = (interfGeom->GetVertex(dirId)->x - interfGeom->GetVertex(baseId)->x);
			pathDY = (interfGeom->GetVertex(dirId)->y - interfGeom->GetVertex(baseId)->y);
			pathDZ = (interfGeom->GetVertex(dirId)->z - interfGeom->GetVertex(baseId)->z);

			pathDXInput = fmt::format("{}", pathDX);
			pathDYInput = fmt::format("{}", pathDY);
			pathDZInput = fmt::format("{}", pathDZ);
		}
	}
}

void ImFacetExtrude::FacetCenterButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format("Facet {} center", *foundId + 1);
		Vector3d center3d = interfGeom->GetFacet(*foundId)->sh.center;
		curveX0=(center3d.x);
		curveY0=(center3d.y);
		curveZ0=(center3d.z);

		curveX0Input = fmt::format("{}", curveX0);
		curveY0Input = fmt::format("{}", curveY0);
		curveZ0Input = fmt::format("{}", curveZ0);
	}
}

void ImFacetExtrude::FacetIndex1ButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		size_t vertexId = interfGeom->GetFacet(*foundId)->indices[0];

		facetInfo = fmt::format("Facet {} index1: Vertex {}", *foundId + 1, vertexId);

		curveX0=(interfGeom->GetVertex(vertexId)->x);
		curveY0=(interfGeom->GetVertex(vertexId)->y);
		curveZ0=(interfGeom->GetVertex(vertexId)->z);

		curveX0Input = fmt::format("{}", curveX0);
		curveY0Input = fmt::format("{}", curveY0);
		curveZ0Input = fmt::format("{}", curveZ0);
	}
}

void ImFacetExtrude::CurveGetBaseButtonPress()
{
	if (auto foundId = AssertOneVertexSelected()) {
		facetInfo = fmt::format("Vertex {}", *foundId + 1);

		curveX0=(interfGeom->GetVertex(*foundId)->x);
		curveY0=(interfGeom->GetVertex(*foundId)->y);
		curveZ0=(interfGeom->GetVertex(*foundId)->z);

		curveX0Input = fmt::format("{}", curveX0);
		curveY0Input = fmt::format("{}", curveY0);
		curveZ0Input = fmt::format("{}", curveZ0);
	}
}

void ImFacetExtrude::CurveFacetUButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format(u8"Facet {} u\u20D7", *foundId + 1);

		curveDX=(interfGeom->GetFacet(*foundId)->sh.U.x);
		curveDY=(interfGeom->GetFacet(*foundId)->sh.U.y);
		curveDZ=(interfGeom->GetFacet(*foundId)->sh.U.z);

		curveDXInput = fmt::format("{}", curveDX);
		curveDYInput = fmt::format("{}", curveDY);
		curveDZInput = fmt::format("{}", curveDZ);
	}
}

void ImFacetExtrude::CurveFacetVButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format(u8"Facet {} v\u20D7", *foundId + 1);

		curveDX = (interfGeom->GetFacet(*foundId)->sh.V.x);
		curveDY = (interfGeom->GetFacet(*foundId)->sh.V.y);
		curveDZ = (interfGeom->GetFacet(*foundId)->sh.V.z);

		curveDXInput = fmt::format("{}", curveDX);
		curveDYInput = fmt::format("{}", curveDY);
		curveDZInput = fmt::format("{}", curveDZ);
	}
}

void ImFacetExtrude::CurveGetDirectionButtonPress()
{
	if (auto foundId = AssertOneVertexSelected()) {
		if(Util::getNumber(&curveX0, curveX0Input) && Util::getNumber(&curveY0, curveY0Input) && Util::getNumber(&curveZ0, curveZ0Input)) {
			facetInfo = fmt::format("Vertex {}", *foundId + 1);
			curveDX=(interfGeom->GetVertex(*foundId)->x - curveX0);
			curveDY=(interfGeom->GetVertex(*foundId)->y - curveY0);
			curveDZ=(interfGeom->GetVertex(*foundId)->z - curveZ0);

			curveDXInput = fmt::format("{}", curveDX);
			curveDYInput = fmt::format("{}", curveDY);
			curveDZInput = fmt::format("{}", curveDZ);
		}
	}
}

void ImFacetExtrude::FacetNXButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format("Facet {} N x X", *foundId + 1);
		curveDirFac = *foundId;
		curveDX=0;
		curveDY=(interfGeom->GetFacet(*foundId)->sh.N.z);
		curveDZ=(-interfGeom->GetFacet(*foundId)->sh.N.y);

		curveDXInput = fmt::format("{}", curveDX);
		curveDYInput = fmt::format("{}", curveDY);
		curveDZInput = fmt::format("{}", curveDZ);
	}
}

void ImFacetExtrude::FacetNYButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format("Facet {} N x Y", *foundId + 1);
		curveDirFac = *foundId;
		curveDX = (-interfGeom->GetFacet(*foundId)->sh.N.z);
		curveDY = 0;
		curveDZ = (interfGeom->GetFacet(*foundId)->sh.N.x);

		curveDXInput = fmt::format("{}", curveDX);
		curveDYInput = fmt::format("{}", curveDY);
		curveDZInput = fmt::format("{}", curveDZ);
	}
}

void ImFacetExtrude::FacetNZButtonPress()
{
	if (auto foundId = AssertOneFacetSelected()) {
		facetInfo = fmt::format("Facet {} N x Z", *foundId + 1);
		curveDirFac = *foundId;
		curveDX = (interfGeom->GetFacet(*foundId)->sh.N.y);
		curveDY = (-interfGeom->GetFacet(*foundId)->sh.N.x);
		curveDZ = 0;

		curveDXInput = fmt::format("{}", curveDX);
		curveDYInput = fmt::format("{}", curveDY);
		curveDZInput = fmt::format("{}", curveDZ);
	}
}
