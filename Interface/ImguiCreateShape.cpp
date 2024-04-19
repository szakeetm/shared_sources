#include "ImguiCreateShape.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"

void ImCreateShape::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowSize(ImVec2(txtW * 80, txtH * 20));
	ImGui::Begin("Create shape", &drawn, ImGuiWindowFlags_NoResize);
	if (ImGui::BeginTabBar("Shape selection")) {

		if (ImGui::BeginTabItem("Square / rectangle")) {
			shapeSel = rect;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Curcle / Elipse")) {
			shapeSel = elipse;
			ImGui::EndTabItem();
		}
		if (ImGui::BeginTabItem("Racetrack")) {
			shapeSel = track;
			ImGui::EndTabItem();
		}
		ImGui::EndTabBar();
	}
	ImGui::BeginGroup();
	ImGui::Text("// TODO Place images here");
	ImGui::EndGroup();
	ImGui::BeginChild("Position", ImVec2(0, txtH * 5.5), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Position");

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Center:", txtW*12);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###centerXIn", &centerXIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###centerYIn", &centerYIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###centerZIn", &centerZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet center")) { FacetCenterButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Vertex")) { VertexButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(centerRowMsg);

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Axis1 direction:", txtW*12);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###axis1XIn", &axis1XIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###axis1YIn", &axis1YIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###axis1ZIn", &axis1ZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet U")) { FacetUButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex")) { CenterToVertAx1ButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(axisRowMsg);

	ImGui::AlignTextToFramePadding();
	ImGui::TextWithMargin("Normal direction:", txtW * 12);
	ImGui::SameLine();
	ImGui::Text("X:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###normalXIn", &normalXIn);
	ImGui::SameLine();
	ImGui::Text("Y:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###normalYIn", &normalYIn); ImGui::SameLine();
	ImGui::Text("Z:");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###normalZIn", &normalZIn);
	ImGui::SameLine();
	if (ImGui::Button("Facet N")) { FacetNButtonPress(); }
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex")) { CenterToVertNormalButtonPress(); }
	ImGui::SameLine();
	ImGui::Text(normalRowMsg);

	ImGui::EndChild();
	ImGui::BeginChild("Size", ImVec2(0, txtH * 4.5), ImGuiChildFlags_Border);
	ImGui::TextDisabled("Size");

	ImGui::TextWithMargin("Axis1 length:", txtW * 10);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###axis1Len", &axis1LenIn);
	ImGui::SameLine();
	ImGui::TextWithMargin("cm", txtW * 3);
	if (shapeSel == track) {
		ImGui::SameLine();
		ImGui::TextWithMargin("Racetrack top length:", txtW * 15);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(txtW * 6);
		ImGui::InputText("###trackTopLenIn", &trackTopLenIn);
		ImGui::SameLine();
		ImGui::TextWithMargin("cm", txtW * 3);
		ImGui::SameLine();
		if (ImGui::Button("Full circle sides")) { FullCircleSidesButtonPress(); };
	}

	ImGui::TextWithMargin("Axis2 length:", txtW * 10);
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###axis2Len", &axis1LenIn);
	ImGui::SameLine();
	ImGui::TextWithMargin("cm", txtW * 3);
	if (shapeSel != rect) {
		ImGui::SameLine();
		ImGui::TextWithMargin("Steps in arc:", txtW * 15);
		ImGui::SameLine();
		ImGui::SetNextItemWidth(txtW * 6);
		ImGui::InputText("###arcStepsIn", &arcStepsIn);
	}
	ImGui::EndChild();
	if (ImGui::Button("Create facet")) {
		ApplyButtonPress();
	}
	ImGui::End();
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

	centerXIn = fmt::format("{}", centerX);
	centerYIn = fmt::format("{}", centerY);
	centerZIn = fmt::format("{}", centerZ);

	centerRowMsg = fmt::format("Center of facet {}", selFacets[0] + 1);
}

void ImCreateShape::VertexButtonPress()
{
}

void ImCreateShape::FacetUButtonPress()
{
}

void ImCreateShape::CenterToVertAx1ButtonPress()
{
}

void ImCreateShape::FacetNButtonPress()
{
}

void ImCreateShape::CenterToVertNormalButtonPress()
{
}

void ImCreateShape::FullCircleSidesButtonPress()
{
}

void ImCreateShape::ApplyButtonPress()
{
}
