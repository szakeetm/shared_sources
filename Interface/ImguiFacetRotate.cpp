#include "ImguiFacetRotate.h"
#include "ImguiPopup.h"
#include "Facet_shared.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "ImguiExtensions.h"
#include "Helper/StringHelper.h"

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
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				if (ImGui::RadioButton("Normal vector", mode == facetN)) mode = facetN;
				ImGui::EndTable();
			}
		}
		ImGui::EndChild();
		if (ImGui::RadioButton("Define by 2 verticies", mode == verticies)) mode = verticies;
		if (ImGui::RadioButton("Define by equation:", mode == equation)) mode = equation;
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

			}
			ImGui::EndTable();
		}
	}
	ImGui::EndChild();
	ImGui::InputTextLLabel("Degrees:", &degIn, 0, txtW * 6); ImGui::SameLine();
	ImGui::InputTextLLabel("Radians:", &radIn, 0, txtW * 6);

	if (ImGui::Button("Rotate facet")) {
		RotateFacetButtonPress(false);
	}
	ImGui::SameLine();
	if (ImGui::Button("Copy facet")) {
		RotateFacetButtonPress(true);
	}
	ImGui::End();
}

void ImFacetRotate::RotateFacetButtonPress(bool copy)
{
}

void ImFacetRotate::DoRotate(bool copy)
{
}
