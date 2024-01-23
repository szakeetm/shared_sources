#include "ImguiFacetExtrude.h"
#include "ImguiExtensions.h"
#include "imgui_stdlib/imgui_stdlib.h"

void ImFacetExtrude::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowSize(ImVec2(txtW * 50, txtH * 30.5));
	ImGui::SetNextWindowPos(ImVec2(txtW * 5, txtH * 3), ImGuiCond_FirstUseEver);
	ImGui::Begin("Extrude Facet", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize);

	ImGui::BeginChild("###EF1", ImVec2(0,txtH*4.25), true);
	ImGui::TextDisabled("Towards/against normal");
	if (ImGui::RadioButton("Towards normal", mode == facetNormal)) mode = facetNormal;
	ImGui::SameLine();
	if (ImGui::RadioButton("Against normal", mode == facetAntinormal)) mode = facetAntinormal;
	ImGui::InputTextLLabel("extrusion length:", &facetLengthInput, 0, txtW * 6);
	ImGui::EndChild();

	ImGui::BeginChild("###EF2", ImVec2(0, txtH * 6), true);
	ImGui::TextDisabled("Along straight path");
	if (ImGui::RadioButton("Direction vector", mode == directionVector)) mode = directionVector;
	ImGui::Text("dX:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DX", &pathDXInput); ImGui::SameLine();
	ImGui::Text("cm dY:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DY", &pathDYInput); ImGui::SameLine();
	ImGui::Text("cm dZ:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF2DZ", &pathDZInput); ImGui::SameLine();
	ImGui::Text("cm");
	if (ImGui::Button("Get Base Vertex")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Get Dir. Vertex")) {

	}
	ImGui::EndChild();

	ImGui::BeginChild("###EF3", ImVec2(0, txtH * 16.75), true);
	ImGui::TextDisabled("Along curve");
	if (ImGui::RadioButton("Towards normal", mode == curveNormal)) mode = curveNormal;
	ImGui::SameLine();
	if (ImGui::RadioButton("Against normal", mode == curveAntinormal)) mode = curveAntinormal;
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
	ImGui::Text("cm Y0:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3Y0", &curveY0Input); ImGui::SameLine();
	ImGui::Text("cm Z0:"); ImGui::SameLine();
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
	ImGui::Text("cm dY:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DY", &curvedYInput); ImGui::SameLine();
	ImGui::Text("cm dZ:"); ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3DZ", &curveDZInput); ImGui::SameLine();
	ImGui::Text("cm");

	ImGui::InputTextLLabel("Radius:", &radiusInput, 0, txtW*6);
	ImGui::SameLine();
	ImGui::Text("cm");
	ImGui::InputTextLLabel("Total angle:", &angleDegInput, 0, txtW * 6);
	ImGui::SameLine();
	ImGui::Text("deg");
	ImGui::SameLine();
	ImGui::SetNextItemWidth(txtW * 6);
	ImGui::InputText("###EF3RAD", &angleRadInput); ImGui::SameLine();
	ImGui::Text("rad");
	ImGui::InputTextLLabel("Total length:", &curveLengthInput, 0, txtW * 6);
	ImGui::SameLine();
	ImGui::Text("cm");
	ImGui::InputTextLLabel("Steps:", &curveLengthInput, 0, txtW * 6);

	ImGui::EndChild();

	if (ImGui::Button("Execute")) {

	}

	ImGui::End();
}
