#include "ImguiFacetScale.h"
#include "ImguiExtensions.h"

void ImFacetScale::Draw()
{
	if (!drawn) return;
	ImGui::SetNextWindowPos(ImVec2(5 * txtW, 3 * txtH), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowSize(ImVec2(txtW * 55, txtH * 14), ImGuiCond_FirstUseEver);
	ImGui::Begin("Scale selected facets", &drawn, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoResize );

	ImGui::BeginChild("##FSC1", ImVec2(0, 6 * txtH), true);
	ImGui::TextDisabled("Invariant point definition mode");
	if (ImGui::RadioButton("", invPt == Coords)) invPt = Coords;
	if (invPt != Coords) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel("X=", &invariantPointInput[X], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Y=", &invariantPointInput[Y], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Z=", &invariantPointInput[Z], 0, txtW * 6);
	if (invPt != Coords) ImGui::EndDisabled();

	if (ImGui::RadioButton("Selected vertex", invPt == Vertex)) invPt = Vertex;
	if (ImGui::RadioButton("Center of selected facet", invPt == Facet)) invPt = Facet;
	if (invPt != Facet) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel("#", &facetIdInput);
	ImGui::SameLine();
	if (ImGui::Button("<-Get selected")) {

	}
	if (invPt != Facet) ImGui::EndDisabled();

	ImGui::EndChild();
	ImGui::BeginChild("##FSC2", ImVec2(0, 4.5 * txtH), true);
	ImGui::TextDisabled("Scale factor");
	if (ImGui::RadioButton("Uniform", scaleFac==Uniform)) scaleFac=Uniform;
	if (scaleFac != Uniform) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel(" ", &uniformScaleNumeratorInput);
	ImGui::SameLine();
	ImGui::InputTextLLabel("(=1/", &uniformScaleDenominatorInput);
	ImGui::SameLine(); ImGui::Text(")");
	if (scaleFac != Uniform) ImGui::EndDisabled();
	if (ImGui::RadioButton("Distorted", scaleFac==Distorted)) scaleFac=Distorted;
	if (scaleFac != Distorted) ImGui::BeginDisabled();
	ImGui::SameLine();
	ImGui::InputTextLLabel("X:", &distortedScaleFactorInput[X], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Y:", &distortedScaleFactorInput[Y], 0, txtW * 6);
	ImGui::SameLine();
	ImGui::InputTextLLabel("Z:", &distortedScaleFactorInput[Z], 0, txtW * 6);
	if (scaleFac != Distorted) ImGui::EndDisabled();
	ImGui::EndChild();

	if (ImGui::Button("Scale facet")) {

	}
	ImGui::SameLine();
	if (ImGui::Button("Copy facet")) {

	}
	ImGui::End();
}

void ImFacetScale::GetSelectedFacetButtonPress()
{
}

void ImFacetScale::ScaleFacetButtonPress()
{
}

void ImFacetScale::CopyFacetButtonPress()
{
}
