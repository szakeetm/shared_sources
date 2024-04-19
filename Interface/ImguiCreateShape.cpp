#include "ImguiCreateShape.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"

void ImCreateShape::Draw()
{
	if (!drawn) return;

	ImGui::SetNextWindowSizeConstraints(ImVec2(txtW * 70, txtH * 20), ImVec2(txtW * 700, txtH * 200));
	ImGui::Begin("Create shape", &drawn);
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
	ImGui::Text("Images to be placed here");
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
	if(ImGui::Button("Facet center")) {}
	ImGui::SameLine();
	if(ImGui::Button("Vertex")) {}

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
	if (ImGui::Button("Facet U")) {}
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex")) {}

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
	if (ImGui::Button("Facet N")) {}
	ImGui::SameLine();
	if (ImGui::Button("Center to vertex")) {}

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
		if (ImGui::Button("Full circle sides")) {};
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

	}
	ImGui::End();
}
