#include "ImguiFacetExtrude.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"
#include "Helper/StringHelper.h"
#include "Helper/MathTools.h"

void ImFacetExtrude::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 31));
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

	ImGui::BeginChild("###EF2", ImVec2(0, txtH * 6), true);
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

	}
	ImGui::SameLine();
	if (ImGui::Button("Get Dir. Vertex")) {

	}
	if (!(mode == directionVector)) ImGui::EndDisabled();
	ImGui::EndChild();

	ImGui::BeginChild("###EF3", ImVec2(0, txtH * 17), true);
	ImGui::TextDisabled("Along curve");
	if (ImGui::RadioButton("Towards normal", mode == curveNormal)) mode = curveNormal;
	ImGui::SameLine();
	if (ImGui::RadioButton("Against normal", mode == curveAntinormal)) mode = curveAntinormal;

	if (!(mode == curveNormal || mode == curveAntinormal)) ImGui::BeginDisabled();
	ImGui::Text("Radius base:");
	if (ImGui::Button("Facet center")) {

	} ImGui::SameLine();
	if (ImGui::Button("Facet index1")) {

	} ImGui::SameLine();
	if (ImGui::Button("From selected")) {

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

	} ImGui::SameLine();
	if (ImGui::Button("Facet V")) {

	} ImGui::SameLine();
	if (ImGui::Button("Towards selected")) {

	}
	
	if (ImGui::Button("Facet N x X")) {

	} ImGui::SameLine();
	if (ImGui::Button("Facet N x Y")) {

	} ImGui::SameLine();
	if (ImGui::Button("Facet N x Z")) {

	}

	ImGui::Text("dX:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DX", &curvedXInput); ImGui::SameLine();
	ImGui::Text("cm\tdY:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DY", &curvedYInput); ImGui::SameLine();
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

	}

	ImGui::End();
}

void ImFacetExtrude::ExtrudeButtonPress()
{
}

void ImFacetExtrude::GetBaseButtonPress()
{
}

void ImFacetExtrude::GetDirectionButtonPress()
{
}

void ImFacetExtrude::FacetCenterButtonPress()
{
}

void ImFacetExtrude::FacetIndex1ButtonPress()
{
}

void ImFacetExtrude::CurveGetBaseButtonPress()
{
}

void ImFacetExtrude::CurveFacetUButtonPress()
{
}

void ImFacetExtrude::CurveFacetVButtonPress()
{
}

void ImFacetExtrude::CurveGetDirectionButtonPress()
{
}

void ImFacetExtrude::FacetNXButtonPress()
{
}

void ImFacetExtrude::FacetNYButtonPress()
{
}

void ImFacetExtrude::FacetNZButtonPress()
{
}
