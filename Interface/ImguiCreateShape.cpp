#include "ImguiCreateShape.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"
#include "Interface.h"

void ImCreateShape::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowSize(ImVec2(txtW * 90, txtH * 24.5));
	ImGui::Begin("Create shape", &drawn, ImGuiWindowFlags_NoResize);
	if (ImGui::BeginTabBar("Shape selection")) {

		if (ImGui::BeginTabItem("Square / rectangle###SR")) {
			shapeSel = rect;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Circle / Elipse###CE")) {
			shapeSel = elipse;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Racetrack###RT")) {
			shapeSel = track;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::BeginGroup();
	switch (shapeSel) {
	case rect:
		rectImg->Draw();
		break;
	case elipse:
		elipseImg->Draw();
		break;
	case track:
		trackImg->Draw();
		break;
	}
	ImGui::EndGroup();
	ImGui::BeginChild("Position", ImVec2(0, txtH * 5.5), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Position");
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(2,2));

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Center:", txtW*13);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###centerXIn", &centerXIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###centerYIn", &centerYIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###centerZIn", &centerZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet center")) { FacetCenterButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Vertex")) { VertexButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(centerRowMsg);

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Axis1 direction:", txtW*13);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###axis1XIn", &axis1XIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###axis1YIn", &axis1YIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###axis1ZIn", &axis1ZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet U")) { FacetUButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex##AX")) { CenterToVertAx1ButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(axisRowMsg);

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Normal direction:", txtW * 13);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###normalXIn", &normalXIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###normalYIn", &normalYIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###normalZIn", &normalZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet N")) { FacetNButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex##N")) { CenterToVertNormalButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(normalRowMsg);

	ImGui::PopStyleVar();

	ImGui::EndChild();
	ImGui::BeginChild("Size", ImVec2(0, txtH * 4.5), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Size");

	ImGui::TextWithMargin("Axis1 length:", txtW * 10);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###axis1Len", &axis1LenIn);
	ImGui::SameLine();
	ImGui::TextWithMargin("cm", txtW * 3);
	if (shapeSel == track) {
		ImGui::SameLine();
		ImGui::TextWithMargin("Racetrack top length:", txtW * 15);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(txtW * 10);
		ImGui::InputText("###trackTopLenIn", &trackTopLenIn);
		ImGui::SameLine();
		ImGui::TextWithMargin("cm", txtW * 3);
		ImGui::SameLine();
		if (ImGui::Button("Full circle sides")) { FullCircleSidesButtonPress(); };
	}

	ImGui::TextWithMargin("Axis2 length:", txtW * 10);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 10);
	ImGui::InputText("###axis2Len", &axis2LenIn);
	ImGui::SameLine();
	ImGui::TextWithMargin("cm", txtW * 3);
	if (shapeSel != rect) {
		ImGui::SameLine();
		ImGui::TextWithMargin("Steps in arc:", txtW * 15);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(txtW * 10);
		ImGui::InputText("###arcStepsIn", &arcStepsIn);
	}
	ImGui::EndChild();
	if (ImGui::Button("Create facet")) {
		ApplyButtonPress();
	}
	ImGui::End();
}

void ImCreateShape::OnShow()
{
	if (rectImg == nullptr) rectImg = new ImImage("images/edit_rectangle_transparent.png");
	if (elipseImg == nullptr) elipseImg = new ImImage("images/edit_circle_transparent.png");
	if (trackImg == nullptr) trackImg = new ImImage("images/edit_racetrack_transparent.png");
}

void ImCreateShape::FacetCenterButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	auto selFacets = interfGeom->GetSelectedFacets();
	Vector3d facetCenter = interfGeom->GetFacet(selFacets[0])->sh.center;

	centerX = facetCenter.x;
	centerY = facetCenter.y;
	centerZ = facetCenter.z;

	centerXIn = fmt::format("{:.5g}", centerX);
	centerYIn = fmt::format("{:.5g}", centerY);
	centerZIn = fmt::format("{:.5g}", centerZ);

	centerRowMsg = fmt::format("Center of facet {}", selFacets[0] + 1);
}

void ImCreateShape::VertexButtonPress()
{
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	auto selVertices = interfGeom->GetSelectedVertices();
	Vector3d center = *(interfGeom->GetVertex(selVertices[0]));

	centerX = center.x;
	centerY = center.y;
	centerZ = center.z;

	centerXIn = fmt::format("{:.5g}", centerX);
	centerYIn = fmt::format("{:.5g}", centerY);
	centerZIn = fmt::format("{:.5g}", centerZ);

	centerRowMsg = fmt::format("Vertex {}", selVertices[0] + 1);
}

void ImCreateShape::FacetUButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	auto selFacets = interfGeom->GetSelectedFacets();
	Vector3d facetU = interfGeom->GetFacet(selFacets[0])->sh.U;

	axis1X = facetU.x;
	axis1Y = facetU.y;
	axis1Z = facetU.z;

	axis1XIn = fmt::format("{:.5g}", axis1X);
	axis1YIn = fmt::format("{:.5g}", axis1Y);
	axis1ZIn = fmt::format("{:.5g}", axis1Z);

	axisRowMsg = fmt::format(u8"Facet {} u\u20D7", selFacets[0] + 1);
}

void ImCreateShape::CenterToVertAx1ButtonPress()
{
	if (!Util::getNumber(&centerX, centerXIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center X coordinate");
		return;
	}
	if (!Util::getNumber(&centerY, centerYIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Y coordinate");
		return;
	}
	if (!Util::getNumber(&centerZ, centerZIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Z coordinate");
		return;
	}
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	auto selVertices = interfGeom->GetSelectedVertices();
	Vector3d vertexLocation = *(interfGeom->GetVertex(selVertices[0]));

	axis1X = vertexLocation.x - centerX;
	axis1Y = vertexLocation.y - centerY;
	axis1Z = vertexLocation.z - centerZ;

	axis1XIn = fmt::format("{:.5g}", axis1X);
	axis1YIn = fmt::format("{:.5g}", axis1Y);
	axis1ZIn = fmt::format("{:.5g}", axis1Z);

	axisRowMsg = fmt::format("Center to vertex {}", selVertices[0] + 1);
}

void ImCreateShape::FacetNButtonPress()
{
	if (interfGeom->GetNbSelectedFacets() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one facet");
		return;
	}
	auto selFacets = interfGeom->GetSelectedFacets();
	Vector3d facetN = interfGeom->GetFacet(selFacets[0])->sh.N;

	normalX = facetN.x;
	normalY = facetN.y;
	normalZ = facetN.z;

	normalXIn = fmt::format("{:.5g}", normalX);
	normalYIn = fmt::format("{:.5g}", normalY);
	normalZIn = fmt::format("{:.5g}", normalZ);

	normalRowMsg = fmt::format(u8"Facet {} normal", selFacets[0] + 1);
}

void ImCreateShape::CenterToVertNormalButtonPress()
{
	if (!Util::getNumber(&centerX, centerXIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center X coordinate");
		return;
	}
	if (!Util::getNumber(&centerY, centerYIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Y coordinate");
		return;
	}
	if (!Util::getNumber(&centerZ, centerZIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Z coordinate");
		return;
	}
	if (interfGeom->GetNbSelectedVertex() != 1) {
		ImIOWrappers::InfoPopup("Error", "Select exactly one vertex");
		return;
	}
	auto selVertices = interfGeom->GetSelectedVertices();
	Vector3d vertexLocation = *(interfGeom->GetVertex(selVertices[0]));

	normalX = vertexLocation.x - centerX;
	normalY = vertexLocation.y - centerY;
	normalZ = vertexLocation.z - centerZ;

	normalXIn = fmt::format("{:.5g}", normalX);
	normalYIn = fmt::format("{:.5g}", normalY);
	normalZIn = fmt::format("{:.5g}", normalZ);

	normalRowMsg = fmt::format("Center to vertex {}", selVertices[0] + 1);
}

void ImCreateShape::FullCircleSidesButtonPress()
{
	if (!Util::getNumber(&axis1Len, axis1LenIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 length");
		return;
	}
	if (!Util::getNumber(&axis2Len, axis2LenIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis2 length");
		return;
	}
	trackTopLen = axis1Len - axis2Len;

	trackTopLenIn = fmt::format("{}", trackTopLen);
}

void ImCreateShape::ApplyButtonPress()
{
	// center
	if (!Util::getNumber(&centerX, centerXIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center X coordinate");
		return;
	}
	if (!Util::getNumber(&centerY, centerYIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Y coordinate");
		return;
	}
	if (!Util::getNumber(&centerZ, centerZIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid center Z coordinate");
		return;
	}
	// axis1
	if (!Util::getNumber(&axis1X, axis1XIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 X coordinate");
		return;
	}
	if (!Util::getNumber(&axis1Y, axis1YIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 Y coordinate");
		return;
	}
	if (!Util::getNumber(&axis1Z, axis1ZIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 Z coordinate");
		return;
	}
	// normal
	if (!Util::getNumber(&normalX, normalXIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid normal X coordinate");
		return;
	}
	if (!Util::getNumber(&normalY, normalYIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid normal Y coordinate");
		return;
	}
	if (!Util::getNumber(&normalZ, normalZIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid normal Z coordinate");
		return;
	}
	// null-vectors
	Vector3d center, axisDir, normalDir;
	center.x = centerX;
	center.y = centerY;
	center.z = centerZ;
	axisDir.x = axis1X;
	axisDir.y = axis1Y;
	axisDir.z = axis1Z;
	normalDir.x = normalX;
	normalDir.y = normalY;
	normalDir.z = normalZ;

	if (IsEqual(axisDir.Norme(), 0.0)) {
		ImIOWrappers::InfoPopup("Error", "Axis1 direction can't be null-vector");
		return;
	}
	if (IsEqual(normalDir.Norme(), 0.0)) {
		ImIOWrappers::InfoPopup("Error", "Normal direction can't be null-vector");
		return;
	}
	// axis lengths
	if (!Util::getNumber(&axis1Len, axis1LenIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 length");
		return;
	}
	if (axis1Len <= 0.0) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis1 length");
		return;
	}
	if (!Util::getNumber(&axis2Len, axis2LenIn)) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis2 length");
		return;
	}
	if (axis2Len <= 0.0) {
		ImIOWrappers::InfoPopup("Error", "Invalid axis2 length");
		return;
	}
	if (shapeSel == track) {
		if (!Util::getNumber(&trackTopLen, trackTopLenIn)) {
			ImIOWrappers::InfoPopup("Error", "Can't parse racetrack top length");
			return;
		}
		if (trackTopLen >= axis1Len) {
			ImIOWrappers::InfoPopup("Error", "For a racetrack, the top length must be less than Axis1");
			return;
		}
		if (!(trackTopLen >= (axis1Len - axis2Len))) {
			ImIOWrappers::InfoPopup("Error", "For a racetrack, the top length must be at least (Axis1 - Axis2)");
			return;
		}
		if (trackTopLen <= 0) {
			ImIOWrappers::InfoPopup("Error", "Top length must be positive");
			return;
		}
	}
	if (shapeSel == track || shapeSel == elipse) {
		if (!Util::getNumber(&arcSteps, arcStepsIn)) {
			ImIOWrappers::InfoPopup("Error", "Can't parse steps in arc");
			return;
		}
		if (arcSteps<2) {
			ImIOWrappers::InfoPopup("Error", "Number of arc steps must be at least 2");
			return;
		}
	}
	{
		LockWrapper lW(mApp->imguiRenderLock);
		if (!mApp->AskToReset()) return;
		switch (shapeSel) {
		case rect:
			interfGeom->CreateRectangle(center, axisDir, normalDir, axis1Len, axis2Len);
			break;
		case elipse:
			interfGeom->CreateCircle(center, axisDir, normalDir, axis1Len, axis2Len, (size_t)arcSteps);
			break;
		case track:
			interfGeom->CreateRacetrack(center, axisDir, normalDir, axis1Len, axis2Len, trackTopLen, (size_t)arcSteps);
			break;
		default:
			break;
		}
		mApp->worker.MarkToReload();
		mApp->changedSinceSave = true;
		mApp->UpdateFacetlistSelected();
	}
}